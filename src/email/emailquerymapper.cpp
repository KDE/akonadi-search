/*
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "emailquerymapper.h"
#include "emailquerypropertymapper.h"
#include "querymapper_p.h"
#include "akonadisearch_debug.h"

#include <QVector>
#include <QVariant>
#include <QDateTime>

#include <Akonadi/KMime/MessageFlags>
#include <KMime/Message>

using namespace Akonadi::Search;

EmailQueryMapper::EmailQueryMapper()
{
}

QStringList EmailQueryMapper::mimeTypes()
{
    return { KMime::Message::mimeType() };
}

const QueryPropertyMapper &EmailQueryMapper::propertyMapper()
{
    return EmailQueryPropertyMapper::instance();
}

Xapian::Query EmailQueryMapper::recursiveTermMapping(const SearchTerm &term)
{
    // qCDebug(AKONADIPLUGIN_INDEXER_LOG) << term.key() << term.value();
    const EmailSearchTerm::EmailSearchField field = EmailSearchTerm::fromKey(term.key());
    switch (field) {
    case EmailSearchTerm::Message: {
        Xapian::Query q(Xapian::Query::OP_OR,
                        constructQuery(propertyMapper(), Akonadi::EmailSearchTerm::Body, term.value(), term.condition()),
                        constructQuery(propertyMapper(), Akonadi::EmailSearchTerm::Headers, term.value(), term.condition()));
        return negateQuery(q, term.isNegated());
    }
    case Akonadi::EmailSearchTerm::Body:
    case Akonadi::EmailSearchTerm::Headers:
    case Akonadi::EmailSearchTerm::ByteSize:
    case Akonadi::EmailSearchTerm::Subject:
    case Akonadi::EmailSearchTerm::HeaderFrom:
    case Akonadi::EmailSearchTerm::HeaderTo:
    case Akonadi::EmailSearchTerm::HeaderCC:
    case Akonadi::EmailSearchTerm::HeaderBCC:
    case Akonadi::EmailSearchTerm::HeaderReplyTo:
    case Akonadi::EmailSearchTerm::HeaderOrganization:
    case Akonadi::EmailSearchTerm::HeaderListId:
    case Akonadi::EmailSearchTerm::HeaderResentFrom:
    case Akonadi::EmailSearchTerm::HeaderXLoop:
    case Akonadi::EmailSearchTerm::HeaderXMailingList:
    case Akonadi::EmailSearchTerm::HeaderXSpamFlag:
    case Akonadi::EmailSearchTerm::Attachment:
        return constructQuery(propertyMapper(), field, term);
    case Akonadi::EmailSearchTerm::HeaderDate: {
        const auto q = constructQuery(propertyMapper(), field,
                                        QString::number(term.value().toDateTime().toSecsSinceEpoch()),
                                        term.condition());
        return negateQuery(q, term.isNegated());
    }
    case Akonadi::EmailSearchTerm::HeaderOnlyDate: {
        const auto q = constructQuery(propertyMapper(), field,
                                        QString::number(QDateTime(term.value().toDate(), {}).toSecsSinceEpoch()),
                                        term.condition());
        return negateQuery(q, term.isNegated());
    }

    case Akonadi::EmailSearchTerm::MessageStatus:
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Flagged)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::IsImportantFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::ToAct)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::IsToActFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Watched)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::IsWatchedFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Deleted)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::IsDeletedFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Spam)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::IsSpamFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Replied)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::IsRepliedFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Ignored)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::IsIgnoredFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Forwarded)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::IsForwardedFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Sent)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::IsSentFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Queued)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::IsQueuedFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Ham)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::IsHamFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Seen)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::IsReadFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::HasAttachment)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::HasAttachmentFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Encrypted)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::IsEncryptedFlag, !term.isNegated());
        }
        if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::HasInvitation)) {
            return constructQuery(propertyMapper(), EmailQueryPropertyMapper::HasInvitationFlag, !term.isNegated());
        }
        break;
    case Akonadi::EmailSearchTerm::MessageTag:
        //search directly in akonadi? or index tags.
        return {};
    default:
        return QueryMapper::recursiveTermMapping(term);
    }

    return {};
}
