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
    // TODO: Cache all this somehow
    mPropMapper.insertPrefix(QStringLiteral("from"), QStringLiteral("F"));
    mPropMapper.insertPrefix(QStringLiteral("to"), QStringLiteral("T"));
    mPropMapper.insertPrefix(QStringLiteral("cc"), QStringLiteral("CC"));
    mPropMapper.insertPrefix(QStringLiteral("bcc"), QStringLiteral("BC"));
    mPropMapper.insertPrefix(QStringLiteral("subject"), QStringLiteral("SU"));
    mPropMapper.insertPrefix(QStringLiteral("collection"), QStringLiteral("C"));
    mPropMapper.insertPrefix(QStringLiteral("replyto"), QStringLiteral("RT"));
    mPropMapper.insertPrefix(QStringLiteral("organization"), QStringLiteral("O"));
    mPropMapper.insertPrefix(QStringLiteral("listid"), QStringLiteral("LI"));
    mPropMapper.insertPrefix(QStringLiteral("resentfrom"), QStringLiteral("RF"));
    mPropMapper.insertPrefix(QStringLiteral("xloop"), QStringLiteral("XL"));
    mPropMapper.insertPrefix(QStringLiteral("xmailinglist"), QStringLiteral("XML"));
    mPropMapper.insertPrefix(QStringLiteral("xspamflag"), QStringLiteral("XSF"));
    mPropMapper.insertPrefix(QStringLiteral("body"), QStringLiteral("BO"));
    mPropMapper.insertPrefix(QStringLiteral("headers"), QStringLiteral("HE"));

    // TODO: Add body flag?
    // TODO: Add tags?

    // Boolean Flags
    mPropMapper.insertPrefix(QStringLiteral("isimportant"), QStringLiteral("I"));
    mPropMapper.insertPrefix(QStringLiteral("istoact"), QStringLiteral("T"));
    mPropMapper.insertPrefix(QStringLiteral("iswatched"), QStringLiteral("W"));
    mPropMapper.insertPrefix(QStringLiteral("isdeleted"), QStringLiteral("D"));
    mPropMapper.insertPrefix(QStringLiteral("isspam"), QStringLiteral("S"));
    mPropMapper.insertPrefix(QStringLiteral("isreplied"), QStringLiteral("E"));
    mPropMapper.insertPrefix(QStringLiteral("isignored"), QStringLiteral("G"));
    mPropMapper.insertPrefix(QStringLiteral("isforwarded"), QStringLiteral("F"));
    mPropMapper.insertPrefix(QStringLiteral("issent"), QStringLiteral("N"));
    mPropMapper.insertPrefix(QStringLiteral("isqueued"), QStringLiteral("Q"));
    mPropMapper.insertPrefix(QStringLiteral("isham"), QStringLiteral("H"));
    mPropMapper.insertPrefix(QStringLiteral("isread"), QStringLiteral("R"));
    mPropMapper.insertPrefix(QStringLiteral("hasattachment"), QStringLiteral("A"));
    mPropMapper.insertPrefix(QStringLiteral("isencrypted"), QStringLiteral("C"));
    mPropMapper.insertPrefix(QStringLiteral("hasinvitation"), QStringLiteral("V"));

    mPropMapper.insertBoolProperty(QStringLiteral("isimportant"));
    mPropMapper.insertBoolProperty(QStringLiteral("istoact"));
    mPropMapper.insertBoolProperty(QStringLiteral("iswatched"));
    mPropMapper.insertBoolProperty(QStringLiteral("isdeleted"));
    mPropMapper.insertBoolProperty(QStringLiteral("isspam"));
    mPropMapper.insertBoolProperty(QStringLiteral("isreplied"));
    mPropMapper.insertBoolProperty(QStringLiteral("isignored"));
    mPropMapper.insertBoolProperty(QStringLiteral("isforwarded"));
    mPropMapper.insertBoolProperty(QStringLiteral("issent"));
    mPropMapper.insertBoolProperty(QStringLiteral("isqueued"));
    mPropMapper.insertBoolProperty(QStringLiteral("isham"));
    mPropMapper.insertBoolProperty(QStringLiteral("isread"));
    mPropMapper.insertBoolProperty(QStringLiteral("hasattachment"));
    mPropMapper.insertBoolProperty(QStringLiteral("isencrypted"));
    mPropMapper.insertBoolProperty(QStringLiteral("hasinvitation"));

    mPropMapper.insertValueProperty(QStringLiteral("date"), 0);
    mPropMapper.insertValueProperty(QStringLiteral("size"), 1);
    mPropMapper.insertValueProperty(QStringLiteral("onlydate"), 2);
}

QStringList EmailQueryMapper::mimeTypes()
{
    return { KMime::Message::mimeType() };
}


Xapian::Query EmailQueryMapper::map(const Akonadi::SearchQuery &akQuery)
{
    auto akTerm = akQuery.term();

    return recursiveTermMapping(akTerm);
}

Xapian::Query EmailQueryMapper::recursiveTermMapping(const SearchTerm &term)
{
    if (!term.subTerms().isEmpty()) {
        QVector<Xapian::Query> sub;
        const auto subTerms = term.subTerms();
        for (const auto &t : subTerms) {
            const auto q = recursiveTermMapping(t);
            if (!q.empty()) {
                sub.push_back(q);
            }
        }
        return Xapian::Query{ mapRelation(term.relation()), sub.cbegin(), sub.cend() };
    } else {
        // qCDebug(AKONADIPLUGIN_INDEXER_LOG) << term.key() << term.value();
        const EmailSearchTerm::EmailSearchField field = EmailSearchTerm::fromKey(term.key());
        switch (field) {
        case EmailSearchTerm::Message: {
            Xapian::Query q(Xapian::Query::OP_OR,
                            constructQuery(mPropMapper, QStringLiteral("body"), term.value(), term.condition()),
                            constructQuery(mPropMapper, QStringLiteral("headers"), term.value(), term.condition()));
            return negateQuery(q, term.isNegated());
        }
        case Akonadi::EmailSearchTerm::Body:
            return constructQuery(mPropMapper, QStringLiteral("body"), term);
        case Akonadi::EmailSearchTerm::Headers:
            return constructQuery(mPropMapper, QStringLiteral("headers"), term);
        case Akonadi::EmailSearchTerm::ByteSize:
            return constructQuery(mPropMapper, QStringLiteral("size"), term);
        case Akonadi::EmailSearchTerm::HeaderDate: {
            const auto q = constructQuery(mPropMapper, QStringLiteral("date"),
                                          QString::number(term.value().toDateTime().toTime_t()),
                                          term.condition());
            return negateQuery(q, term.isNegated());
        }
        case Akonadi::EmailSearchTerm::HeaderOnlyDate: {
            const auto q = constructQuery(mPropMapper, QStringLiteral("onlydate"),
                                          QString::number(term.value().toDate().toJulianDay()),
                                          term.condition());
            return negateQuery(q, term.isNegated());
        }
        case Akonadi::EmailSearchTerm::Subject:
            return constructQuery(mPropMapper, QStringLiteral("subject"), term);
        case Akonadi::EmailSearchTerm::HeaderFrom:
            return constructQuery(mPropMapper, QStringLiteral("from"), term);
        case Akonadi::EmailSearchTerm::HeaderTo:
            return constructQuery(mPropMapper, QStringLiteral("to"), term);
        case Akonadi::EmailSearchTerm::HeaderCC:
            return constructQuery(mPropMapper, QStringLiteral("cc"), term);
        case Akonadi::EmailSearchTerm::HeaderBCC:
            return constructQuery(mPropMapper, QStringLiteral("bcc"), term);
        case Akonadi::EmailSearchTerm::MessageStatus:
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Flagged)) {
                return constructQuery(mPropMapper, QStringLiteral("isimportant"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::ToAct)) {
                return constructQuery(mPropMapper, QStringLiteral("istoact"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Watched)) {
                return constructQuery(mPropMapper, QStringLiteral("iswatched"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Deleted)) {
                return constructQuery(mPropMapper, QStringLiteral("isdeleted"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Spam)) {
                return constructQuery(mPropMapper, QStringLiteral("isspam"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Replied)) {
                return constructQuery(mPropMapper, QStringLiteral("isreplied"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Ignored)) {
                return constructQuery(mPropMapper, QStringLiteral("isignored"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Forwarded)) {
                return constructQuery(mPropMapper, QStringLiteral("isforwarded"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Sent)) {
                return constructQuery(mPropMapper, QStringLiteral("issent"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Queued)) {
                return constructQuery(mPropMapper, QStringLiteral("isqueued"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Ham)) {
                return constructQuery(mPropMapper, QStringLiteral("isham"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Seen)) {
                return constructQuery(mPropMapper, QStringLiteral("isread"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::HasAttachment)) {
                return constructQuery(mPropMapper, QStringLiteral("hasattachment"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Encrypted)) {
                return constructQuery(mPropMapper, QStringLiteral("isencrypted"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::HasInvitation)) {
                return constructQuery(mPropMapper, QStringLiteral("hasinvitation"), !term.isNegated());
            }
            break;
        case Akonadi::EmailSearchTerm::MessageTag:
            //search directly in akonadi? or index tags.
            break;
        case Akonadi::EmailSearchTerm::HeaderReplyTo:
            return constructQuery(mPropMapper, QStringLiteral("replyto"), term);
        case Akonadi::EmailSearchTerm::HeaderOrganization:
            return constructQuery(mPropMapper, QStringLiteral("organization"), term);
        case Akonadi::EmailSearchTerm::HeaderListId:
            return constructQuery(mPropMapper, QStringLiteral("listid"), term);
        case Akonadi::EmailSearchTerm::HeaderResentFrom:
            return constructQuery(mPropMapper, QStringLiteral("resentfrom"), term);
        case Akonadi::EmailSearchTerm::HeaderXLoop:
            return constructQuery(mPropMapper, QStringLiteral("xloop"), term);
        case Akonadi::EmailSearchTerm::HeaderXMailingList:
            return constructQuery(mPropMapper, QStringLiteral("xmailinglist"), term);
        case Akonadi::EmailSearchTerm::HeaderXSpamFlag:
            return constructQuery(mPropMapper, QStringLiteral("xspamflag"), term);
        case Akonadi::EmailSearchTerm::Attachment:
            return constructQuery(mPropMapper, QStringLiteral("hasattachment"), !term.isNegated());
        case Akonadi::EmailSearchTerm::Unknown:
        default:
            qCWarning(AKONADISEARCH_LOG) << "unknown term " << term.key();
        }
    }
    return {};
}
