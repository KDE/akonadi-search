/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2014  Christian Mollekopf <mollekopf@kolabsys.com>
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

#include "searchplugin.h"

#include "query.h"
#include "term.h"
#include "resultiterator.h"

#include <searchquery.h>

#include "akonadiplugin_indexer_debug.h"
#include <Akonadi/KMime/MessageFlags>
#include <KContacts/Addressee>
#include <KContacts/ContactGroup>
#include <QStringList>
#include <QDateTime>

using namespace Akonadi::Search;

static Term::Operation mapRelation(Akonadi::SearchTerm::Relation relation)
{
    if (relation == Akonadi::SearchTerm::RelAnd){
        return Term::And;
    }
    return Term::Or;
}

static Term::Comparator mapComparator(Akonadi::SearchTerm::Condition comparator) {
    if (comparator == Akonadi::SearchTerm::CondContains){
        return Term::Contains;
    }
    if (comparator == Akonadi::SearchTerm::CondGreaterOrEqual){
        return Term::GreaterEqual;
    }
    if (comparator == Akonadi::SearchTerm::CondGreaterThan){
        return Term::Greater;
    }
    if (comparator == Akonadi::SearchTerm::CondEqual){
        return Term::Equal;
    }
    if (comparator == Akonadi::SearchTerm::CondLessOrEqual){
        return Term::LessEqual;
    }
    if (comparator == Akonadi::SearchTerm::CondLessThan){
        return Term::Less;
    }
    return Term::Auto;
}

static Term getTerm(const Akonadi::SearchTerm &term, const QString &property)
{
    Term t(property, term.value().toString(), mapComparator(term.condition()));
    t.setNegation(term.isNegated());
    return t;
}

Term recursiveEmailTermMapping(const Akonadi::SearchTerm &term)
{
    if (!term.subTerms().isEmpty()) {
        Term t(mapRelation(term.relation()));
        Q_FOREACH (const Akonadi::SearchTerm &subterm, term.subTerms()) {
            const Term newTerm = recursiveEmailTermMapping(subterm);
            if (newTerm.isValid()) {
                t.addSubTerm(newTerm);
            }
        }
        return t;
    } else {
        // qCDebug(AKONADIPLUGIN_INDEXER_LOG) << term.key() << term.value();
        const Akonadi::EmailSearchTerm::EmailSearchField field = Akonadi::EmailSearchTerm::fromKey(term.key());
        switch (field) {
        case Akonadi::EmailSearchTerm::Message: {
            Term s(Term::Or);
            s.setNegation(term.isNegated());
            s.addSubTerm(Term(QStringLiteral("body"), term.value(), mapComparator(term.condition())));
            s.addSubTerm(Term(QStringLiteral("headers"), term.value(), mapComparator(term.condition())));
            return s;
        }
        case Akonadi::EmailSearchTerm::Body:
            return getTerm(term, QStringLiteral("body"));
        case Akonadi::EmailSearchTerm::Headers:
            return getTerm(term, QStringLiteral("headers"));
        case Akonadi::EmailSearchTerm::ByteSize:
            return getTerm(term, QStringLiteral("size"));
        case Akonadi::EmailSearchTerm::HeaderDate: {
            Term s(QStringLiteral("date"), QString::number(term.value().toDateTime().toTime_t()), mapComparator(term.condition()));
            s.setNegation(term.isNegated());
            return s;
        }
        case Akonadi::EmailSearchTerm::HeaderOnlyDate: {
            Term s(QStringLiteral("onlydate"), QString::number(term.value().toDate().toJulianDay()), mapComparator(term.condition()));
            s.setNegation(term.isNegated());
            return s;
        }
        case Akonadi::EmailSearchTerm::Subject:
            return getTerm(term, QStringLiteral("subject"));
        case Akonadi::EmailSearchTerm::HeaderFrom:
            return getTerm(term, QStringLiteral("from"));
        case Akonadi::EmailSearchTerm::HeaderTo:
            return getTerm(term, QStringLiteral("to"));
        case Akonadi::EmailSearchTerm::HeaderCC:
            return getTerm(term, QStringLiteral("cc"));
        case Akonadi::EmailSearchTerm::HeaderBCC:
            return getTerm(term, QStringLiteral("bcc"));
        case Akonadi::EmailSearchTerm::MessageStatus:
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Flagged)) {
                return Term(QStringLiteral("isimportant"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::ToAct)) {
                return Term(QStringLiteral("istoact"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Watched)) {
                return Term(QStringLiteral("iswatched"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Deleted)) {
                return Term(QStringLiteral("isdeleted"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Spam)) {
                return Term(QStringLiteral("isspam"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Replied)) {
                return Term(QStringLiteral("isreplied"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Ignored)) {
                return Term(QStringLiteral("isignored"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Forwarded)) {
                return Term(QStringLiteral("isforwarded"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Sent)) {
                return Term(QStringLiteral("issent"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Queued)) {
                return Term(QStringLiteral("isqueued"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Ham)) {
                return Term(QStringLiteral("isham"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Seen)) {
                return Term(QStringLiteral("isread"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::HasAttachment)) {
                return Term(QStringLiteral("hasattachment"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Encrypted)) {
                return Term(QStringLiteral("isencrypted"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::HasInvitation)) {
                return Term(QStringLiteral("hasinvitation"), !term.isNegated());
            }
            break;
        case Akonadi::EmailSearchTerm::MessageTag:
            //search directly in akonadi? or index tags.
            break;
        case Akonadi::EmailSearchTerm::HeaderReplyTo:
            return getTerm(term, QStringLiteral("replyto"));
        case Akonadi::EmailSearchTerm::HeaderOrganization:
            return getTerm(term, QStringLiteral("organization"));
        case Akonadi::EmailSearchTerm::HeaderListId:
            return getTerm(term, QStringLiteral("listid"));
        case Akonadi::EmailSearchTerm::HeaderResentFrom:
            return getTerm(term, QStringLiteral("resentfrom"));
        case Akonadi::EmailSearchTerm::HeaderXLoop:
            return getTerm(term, QStringLiteral("xloop"));
        case Akonadi::EmailSearchTerm::HeaderXMailingList:
            return getTerm(term, QStringLiteral("xmailinglist"));
        case Akonadi::EmailSearchTerm::HeaderXSpamFlag:
            return getTerm(term, QStringLiteral("xspamflag"));
        case Akonadi::EmailSearchTerm::Unknown:
        default:
            qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "unknown term " << term.key();
        }
    }
    return Term();
}

Term recursiveCalendarTermMapping(const Akonadi::SearchTerm &term)
{
    if (!term.subTerms().isEmpty()) {
        Term t(mapRelation(term.relation()));
        Q_FOREACH (const Akonadi::SearchTerm &subterm, term.subTerms()) {
            const Term newTerm = recursiveCalendarTermMapping(subterm);
            if (newTerm.isValid()) {
                t.addSubTerm(newTerm);
            }
        }
        return t;
    } else {
        // qCDebug(AKONADIPLUGIN_INDEXER_LOG) << term.key() << term.value();
        const Akonadi::IncidenceSearchTerm::IncidenceSearchField field = Akonadi::IncidenceSearchTerm::fromKey(term.key());
        switch (field) {
        case Akonadi::IncidenceSearchTerm::Organizer:
            return getTerm(term, QStringLiteral("organizer"));
        case Akonadi::IncidenceSearchTerm::Summary:
            return getTerm(term, QStringLiteral("summary"));
        case Akonadi::IncidenceSearchTerm::Location:
            return getTerm(term, QStringLiteral("location"));
        case Akonadi::IncidenceSearchTerm::PartStatus: {
            Term t(QStringLiteral("partstatus"), term.value().toString(), Term::Equal);
            t.setNegation(term.isNegated());
            return t;
        }
        default:
            qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "unknown term " << term.key();
        }
    }
    return Term();
}

Term recursiveNoteTermMapping(const Akonadi::SearchTerm &term)
{
    if (!term.subTerms().isEmpty()) {
        Term t(mapRelation(term.relation()));
        Q_FOREACH (const Akonadi::SearchTerm &subterm, term.subTerms()) {
            const Term newTerm = recursiveNoteTermMapping(subterm);
            if (newTerm.isValid()) {
                t.addSubTerm(newTerm);
            }
        }
        return t;
    } else {
        // qCDebug(AKONADIPLUGIN_INDEXER_LOG) << term.key() << term.value();
        const Akonadi::EmailSearchTerm::EmailSearchField field = Akonadi::EmailSearchTerm::fromKey(term.key());
        switch (field) {
        case Akonadi::EmailSearchTerm::Subject:
            return getTerm(term, QStringLiteral("subject"));
        case Akonadi::EmailSearchTerm::Body:
            return getTerm(term, QStringLiteral("body"));
        default:
            qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "unknown term " << term.key();
        }
    }
    return Term();
}

Term recursiveContactTermMapping(const Akonadi::SearchTerm &term)
{
    if (!term.subTerms().isEmpty()) {
        Term t(mapRelation(term.relation()));
        Q_FOREACH (const Akonadi::SearchTerm &subterm, term.subTerms()) {
            const Term newTerm = recursiveContactTermMapping(subterm);
            if (newTerm.isValid()) {
                t.addSubTerm(newTerm);
            }
        }
        return t;
    } else {
        // qCDebug(AKONADIPLUGIN_INDEXER_LOG) << term.key() << term.value();
        const Akonadi::ContactSearchTerm::ContactSearchField field = Akonadi::ContactSearchTerm::fromKey(term.key());
        switch (field) {
        case Akonadi::ContactSearchTerm::Name:
            return getTerm(term, QStringLiteral("name"));
        case Akonadi::ContactSearchTerm::Email:
            return getTerm(term, QStringLiteral("email"));
        case Akonadi::ContactSearchTerm::Nickname:
            return getTerm(term, QStringLiteral("nick"));
        case Akonadi::ContactSearchTerm::Uid:
            return getTerm(term, QStringLiteral("uid"));
        case Akonadi::ContactSearchTerm::Unknown:
        default:
            qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "unknown term " << term.key();
        }
    }
    return Term();
}

QSet<qint64> SearchPlugin::search(const QString &akonadiQuery, const QVector<qint64> &collections, const QStringList &mimeTypes)
{
    const Akonadi::SearchQuery searchQuery = Akonadi::SearchQuery::fromJSON(akonadiQuery.toLatin1());
    if (searchQuery.isNull()) {
        qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "invalid query " << akonadiQuery;
        return QSet<qint64>();
    }
    const Akonadi::SearchTerm term = searchQuery.term();

    Query query;
    if (term.subTerms().isEmpty()) {
        qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "empty query";
        return QSet<qint64>();
    }

    Term t;

    if (mimeTypes.contains(QStringLiteral("message/rfc822"))) {
        // qCDebug(AKONADIPLUGIN_INDEXER_LOG) << "mail query";
        query.setType(QStringLiteral("Email"));
        t = recursiveEmailTermMapping(term);
    } else if (mimeTypes.contains(KContacts::Addressee::mimeType()) || mimeTypes.contains(KContacts::ContactGroup::mimeType())) {
        query.setType(QStringLiteral("Contact"));
        t = recursiveContactTermMapping(term);
    } else if (mimeTypes.contains(QStringLiteral("text/x-vnd.akonadi.note"))) {
        query.setType(QStringLiteral("Note"));
        t = recursiveNoteTermMapping(term);
    } else if (mimeTypes.contains(QStringLiteral("application/x-vnd.akonadi.calendar.event")) ||
               mimeTypes.contains(QStringLiteral("application/x-vnd.akonadi.calendar.todo")) ||
               mimeTypes.contains(QStringLiteral("application/x-vnd.akonadi.calendar.journal")) ||
               mimeTypes.contains(QStringLiteral("application/x-vnd.akonadi.calendar.freebusy"))) {
        query.setType(QStringLiteral("Calendar"));
        t = recursiveCalendarTermMapping(term);
    }

    if (t.subTerms().isEmpty()) {
        qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "no terms added";
        return QSet<qint64>();
    }

    if (searchQuery.limit() > 0) {
        query.setLimit(searchQuery.limit());
    }

    //Filter by collection if not empty
    if (!collections.isEmpty()) {
        Term parentTerm(Term::And);
        Term collectionTerm(Term::Or);
        Q_FOREACH (const qint64 col, collections) {
            collectionTerm.addSubTerm(Term(QStringLiteral("collection"), QString::number(col), Term::Equal));
        }
        parentTerm.addSubTerm(collectionTerm);
        parentTerm.addSubTerm(t);

        query.setTerm(parentTerm);
    } else {
        query.setTerm(t);
    }

    QSet<qint64> resultSet;
    // qCDebug(AKONADIPLUGIN_INDEXER_LOG) << query.toJSON();
    ResultIterator iter = query.exec();
    while (iter.next()) {
        const QByteArray id = iter.id();
        const int fid = deserialize("akonadi", id);
        resultSet << fid;
    }
    qCDebug(AKONADIPLUGIN_INDEXER_LOG) << "Got" << resultSet.count() << "results";
    return resultSet;
}
