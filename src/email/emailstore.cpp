/*
 * Copyright (C) 2017  Daniel Vrátil <dvratil@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <xapian.h>

#include "emailstore.h"
#include "emailindexer.h"
#include "emailquerypropertymapper.h"
#include "store_p.h"
#include "xapiandatabase.h"
#include "xapiandocument.h"
#include "utils.h"

#include <KMime/Message>

#include <QDataStream>

#include <unordered_set>

using namespace Akonadi::Search;

EmailStore::EmailStore()
    : Store()
{
    setDbName(QStringLiteral("email"));
}

QStringList EmailStore::mimeTypes()
{
    return { KMime::Message::mimeType() };
}

bool EmailStore::index(qint64 id, const QByteArray &serializedIndex)
{
    if (!d->ensureDb()) {
        return false;
    }

    QDataStream stream(serializedIndex);
    while (!stream.atEnd()) {
        qint64 documentId;
        stream >> documentId;
        if (documentId == -1) {
            documentId = id;
        }

        Xapian::Document newDoc;
        stream >> newDoc;

        bool merge = false;
        for (auto it = newDoc.termlist_begin(), end = newDoc.termlist_end(); it != end; ++it) {
            if (*it == EmailIndexer::MergeFlagsTerm) {
                merge = true;
                break;
            }
        }


        if (merge) {
            auto doc = d->db->document(id);
            if (doc.isValid()) {
                return mergeFlagsOnly(newDoc, doc);
            }
        }

        // Don't chainup to Store::index() for performance reasons as it would have
        // to unserialise the document again
        d->newChange();
        if (!d->db->replaceDocument(static_cast<uint>(documentId), newDoc)) {
            return false;
        }
    }

    return true;
}

bool EmailStore::mergeFlagsOnly(const Xapian::Document &newDoc, const XapianDocument &_oldDoc)
{
    Xapian::Document oldDoc = _oldDoc.xapianDocument();

    static std::unordered_set<std::string> flagTerms;
    if (flagTerms.empty()) {
        const auto &mapper = EmailQueryPropertyMapper::instance();
        const auto termName = [&mapper](EmailQueryPropertyMapper::Flags flag) -> std::string {
            return "B" + mapper.prefix(flag);
        };
        flagTerms = {
            termName(EmailQueryPropertyMapper::IsReadFlag),
            termName(EmailQueryPropertyMapper::HasAttachmentFlag),
            termName(EmailQueryPropertyMapper::IsImportantFlag),
            termName(EmailQueryPropertyMapper::IsWatchedFlag),
            termName(EmailQueryPropertyMapper::IsToActFlag),
            termName(EmailQueryPropertyMapper::IsDeletedFlag),
            termName(EmailQueryPropertyMapper::IsSpamFlag),
            termName(EmailQueryPropertyMapper::IsRepliedFlag),
            termName(EmailQueryPropertyMapper::IsIgnoredFlag),
            termName(EmailQueryPropertyMapper::IsForwardedFlag),
            termName(EmailQueryPropertyMapper::IsSentFlag),
            termName(EmailQueryPropertyMapper::IsQueuedFlag),
            termName(EmailQueryPropertyMapper::IsHamFlag),
            termName(EmailQueryPropertyMapper::IsEncryptedFlag),
            termName(EmailQueryPropertyMapper::HasInvitationFlag)
        };
    }

    auto end = oldDoc.termlist_end();
    for (auto it = oldDoc.termlist_begin(); it != end; ++it) {
        if (flagTerms.find(*it) != flagTerms.cend()) {
            oldDoc.remove_term(*it);
            end = oldDoc.termlist_end();
        }
    }

    for (auto it = newDoc.termlist_begin(), end = newDoc.termlist_end(); it != end; ++it) {
        if (flagTerms.find(*it) != flagTerms.cend()) {
            oldDoc.add_boolean_term(*it);
        }
    }

    d->newChange();
    return d->db->replaceDocument(oldDoc.get_docid(), newDoc);
}
