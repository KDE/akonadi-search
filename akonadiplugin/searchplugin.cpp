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
            s.addSubTerm(Term(QLatin1String("body"), term.value(), mapComparator(term.condition())));
            s.addSubTerm(Term(QLatin1String("headers"), term.value(), mapComparator(term.condition())));
            return s;
        }
        case Akonadi::EmailSearchTerm::Body:
            return getTerm(term, QLatin1String("body"));
        case Akonadi::EmailSearchTerm::Headers:
            return getTerm(term, QLatin1String("headers"));
        case Akonadi::EmailSearchTerm::ByteSize:
            return getTerm(term, QLatin1String("size"));
        case Akonadi::EmailSearchTerm::HeaderDate: {
            Term s(QLatin1String("date"), QString::number(term.value().toDateTime().toTime_t()), mapComparator(term.condition()));
            s.setNegation(term.isNegated());
            return s;
        }
        case Akonadi::EmailSearchTerm::HeaderOnlyDate: {
            Term s(QLatin1String("onlydate"), QString::number(term.value().toDate().toJulianDay()), mapComparator(term.condition()));
            s.setNegation(term.isNegated());
            return s;
        }
        case Akonadi::EmailSearchTerm::Subject:
            return getTerm(term, QLatin1String("subject"));
        case Akonadi::EmailSearchTerm::HeaderFrom:
            return getTerm(term, QLatin1String("from"));
        case Akonadi::EmailSearchTerm::HeaderTo:
            return getTerm(term, QLatin1String("to"));
        case Akonadi::EmailSearchTerm::HeaderCC:
            return getTerm(term, QLatin1String("cc"));
        case Akonadi::EmailSearchTerm::HeaderBCC:
            return getTerm(term, QLatin1String("bcc"));
        case Akonadi::EmailSearchTerm::MessageStatus:
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Flagged)) {
                return Term(QLatin1String("isimportant"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::ToAct)) {
                return Term(QLatin1String("istoact"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Watched)) {
                return Term(QLatin1String("iswatched"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Deleted)) {
                return Term(QLatin1String("isdeleted"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Spam)) {
                return Term(QLatin1String("isspam"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Replied)) {
                return Term(QLatin1String("isreplied"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Ignored)) {
                return Term(QLatin1String("isignored"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Forwarded)) {
                return Term(QLatin1String("isforwarded"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Sent)) {
                return Term(QLatin1String("issent"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Queued)) {
                return Term(QLatin1String("isqueued"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Ham)) {
                return Term(QLatin1String("isham"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Seen)) {
                return Term(QLatin1String("isread"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::HasAttachment)) {
                return Term(QLatin1String("hasattachment"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::Encrypted)) {
                return Term(QLatin1String("isencrypted"), !term.isNegated());
            }
            if (term.value().toString() == QString::fromLatin1(Akonadi::MessageFlags::HasInvitation)) {
                return Term(QLatin1String("hasinvitation"), !term.isNegated());
            }
            break;
        case Akonadi::EmailSearchTerm::MessageTag:
            //search directly in akonadi? or index tags.
            break;
        case Akonadi::EmailSearchTerm::HeaderReplyTo:
            return getTerm(term, QLatin1String("replyto"));
        case Akonadi::EmailSearchTerm::HeaderOrganization:
            return getTerm(term, QLatin1String("organization"));
        case Akonadi::EmailSearchTerm::HeaderListId:
            return getTerm(term, QLatin1String("listid"));
        case Akonadi::EmailSearchTerm::HeaderResentFrom:
            return getTerm(term, QLatin1String("resentfrom"));
        case Akonadi::EmailSearchTerm::HeaderXLoop:
            return getTerm(term, QLatin1String("xloop"));
        case Akonadi::EmailSearchTerm::HeaderXMailingList:
            return getTerm(term, QLatin1String("xmailinglist"));
        case Akonadi::EmailSearchTerm::HeaderXSpamFlag:
            return getTerm(term, QLatin1String("xspamflag"));
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
            return getTerm(term, "organizer");
        case Akonadi::IncidenceSearchTerm::Summary:
            return getTerm(term, "summary");
        case Akonadi::IncidenceSearchTerm::Location:
            return getTerm(term, "location");
        case Akonadi::IncidenceSearchTerm::PartStatus: {
            Term t("partstatus", term.value().toString(), Term::Equal);
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
            return getTerm(term, QLatin1String("subject"));
        case Akonadi::EmailSearchTerm::Body:
            return getTerm(term, QLatin1String("body"));
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
            return getTerm(term, QLatin1String("name"));
        case Akonadi::ContactSearchTerm::Email:
            return getTerm(term, QLatin1String("email"));
        case Akonadi::ContactSearchTerm::Nickname:
            return getTerm(term, QLatin1String("nick"));
        case Akonadi::ContactSearchTerm::Uid:
            return getTerm(term, QLatin1String("uid"));
        case Akonadi::ContactSearchTerm::Unknown:
        default:
            qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "unknown term " << term.key();
        }
    }
    return Term();
}

QSet<qint64> SearchPlugin::search(const QString &akonadiQuery, const QList<qint64> &collections, const QStringList &mimeTypes)
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

    if (mimeTypes.contains(QLatin1String("message/rfc822"))) {
        // qCDebug(AKONADIPLUGIN_INDEXER_LOG) << "mail query";
        query.setType(QLatin1String("Email"));
        t = recursiveEmailTermMapping(term);
    } else if (mimeTypes.contains(KContacts::Addressee::mimeType()) || mimeTypes.contains(KContacts::ContactGroup::mimeType())) {
        query.setType(QLatin1String("Contact"));
        t = recursiveContactTermMapping(term);
    } else if (mimeTypes.contains(QLatin1String("text/x-vnd.akonadi.note"))) {
        query.setType(QLatin1String("Note"));
        t = recursiveNoteTermMapping(term);
    } else if (mimeTypes.contains(QLatin1String("application/x-vnd.akonadi.calendar.event")) ||
               mimeTypes.contains(QLatin1String("application/x-vnd.akonadi.calendar.todo")) ||
               mimeTypes.contains(QLatin1String("application/x-vnd.akonadi.calendar.journal")) ||
               mimeTypes.contains(QLatin1String("application/x-vnd.akonadi.calendar.freebusy"))) {
        query.setType(QLatin1String("Calendar"));
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
            collectionTerm.addSubTerm(Term(QLatin1String("collection"), QString::number(col), Term::Equal));
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
