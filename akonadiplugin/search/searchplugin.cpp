/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "searchplugin.h"

#include "query.h"
#include "resultiterator.h"
#include "term.h"

#include <Akonadi/SearchQuery>

#include "akonadiplugin_indexer_debug.h"
#include <Akonadi/MessageFlags>
#include <KContacts/Addressee>
#include <KContacts/ContactGroup>

using namespace Qt::Literals::StringLiterals;
using namespace Akonadi::Search;

static Term::Operation mapRelation(Akonadi::SearchTerm::Relation relation)
{
    if (relation == Akonadi::SearchTerm::RelAnd) {
        return Term::And;
    }
    return Term::Or;
}

static Term::Comparator mapComparator(Akonadi::SearchTerm::Condition comparator)
{
    if (comparator == Akonadi::SearchTerm::CondContains) {
        return Term::Contains;
    }
    if (comparator == Akonadi::SearchTerm::CondGreaterOrEqual) {
        return Term::GreaterEqual;
    }
    if (comparator == Akonadi::SearchTerm::CondGreaterThan) {
        return Term::Greater;
    }
    if (comparator == Akonadi::SearchTerm::CondEqual) {
        return Term::Equal;
    }
    if (comparator == Akonadi::SearchTerm::CondLessOrEqual) {
        return Term::LessEqual;
    }
    if (comparator == Akonadi::SearchTerm::CondLessThan) {
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

static Term recursiveEmailTermMapping(const Akonadi::SearchTerm &term)
{
    const auto subTermsResult = term.subTerms();
    if (!subTermsResult.isEmpty()) {
        Term t(mapRelation(term.relation()));
        for (const Akonadi::SearchTerm &subterm : subTermsResult) {
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
            s.addSubTerm(Term(u"body"_s, term.value(), mapComparator(term.condition())));
            s.addSubTerm(Term(u"headers"_s, term.value(), mapComparator(term.condition())));
            return s;
        }
        case Akonadi::EmailSearchTerm::Body:
            return getTerm(term, u"body"_s);
        case Akonadi::EmailSearchTerm::Headers:
            return getTerm(term, u"headers"_s);
        case Akonadi::EmailSearchTerm::ByteSize:
            return getTerm(term, u"size"_s);
        case Akonadi::EmailSearchTerm::HeaderDate: {
            Term s(u"date"_s, QString::number(term.value().toDateTime().toSecsSinceEpoch()), mapComparator(term.condition()));
            s.setNegation(term.isNegated());
            return s;
        }
        case Akonadi::EmailSearchTerm::HeaderOnlyDate: {
            Term s(u"onlydate"_s, QString::number(term.value().toDate().toJulianDay()), mapComparator(term.condition()));
            s.setNegation(term.isNegated());
            return s;
        }
        case Akonadi::EmailSearchTerm::Subject:
            return getTerm(term, u"subject"_s);
        case Akonadi::EmailSearchTerm::HeaderFrom:
            return getTerm(term, u"from"_s);
        case Akonadi::EmailSearchTerm::HeaderTo:
            return getTerm(term, u"to"_s);
        case Akonadi::EmailSearchTerm::HeaderCC:
            return getTerm(term, u"cc"_s);
        case Akonadi::EmailSearchTerm::HeaderBCC:
            return getTerm(term, u"bcc"_s);
        case Akonadi::EmailSearchTerm::MessageStatus: {
            const QString value = term.value().toString();
            if (value == QLatin1StringView(Akonadi::MessageFlags::Flagged)) {
                return Term(u"isimportant"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::ToAct)) {
                return Term(u"istoact"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Watched)) {
                return Term(u"iswatched"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Deleted)) {
                return Term(u"isdeleted"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Spam)) {
                return Term(u"isspam"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Replied)) {
                return Term(u"isreplied"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Ignored)) {
                return Term(u"isignored"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Forwarded)) {
                return Term(u"isforwarded"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Sent)) {
                return Term(u"issent"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Queued)) {
                return Term(u"isqueued"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Ham)) {
                return Term(u"isham"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Seen)) {
                return Term(u"isread"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::HasAttachment)) {
                return Term(u"hasattachment"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Encrypted)) {
                return Term(u"isencrypted"_s, !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::HasInvitation)) {
                return Term(u"hasinvitation"_s, !term.isNegated());
            }
            break;
        }
        case Akonadi::EmailSearchTerm::MessageTag:
            // search directly in akonadi? or index tags.
            break;
        case Akonadi::EmailSearchTerm::HeaderReplyTo:
            return getTerm(term, u"replyto"_s);
        case Akonadi::EmailSearchTerm::HeaderOrganization:
            return getTerm(term, u"organization"_s);
        case Akonadi::EmailSearchTerm::HeaderListId:
            return getTerm(term, u"listid"_s);
        case Akonadi::EmailSearchTerm::HeaderResentFrom:
            return getTerm(term, u"resentfrom"_s);
        case Akonadi::EmailSearchTerm::HeaderXLoop:
            return getTerm(term, u"xloop"_s);
        case Akonadi::EmailSearchTerm::HeaderXMailingList:
            return getTerm(term, u"xmailinglist"_s);
        case Akonadi::EmailSearchTerm::HeaderXSpamFlag:
            return getTerm(term, u"xspamflag"_s);
        case Akonadi::EmailSearchTerm::Attachment:
            return Term(u"hasattachment"_s, !term.isNegated());
        case Akonadi::EmailSearchTerm::Unknown:
        default:
            if (!term.key().isEmpty()) {
                qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "unknown term " << term.key();
            }
        }
    }
    return {};
}

static Term recursiveCalendarTermMapping(const Akonadi::SearchTerm &term)
{
    const auto subTerms{term.subTerms()};
    if (!subTerms.isEmpty()) {
        Term t(mapRelation(term.relation()));
        for (const Akonadi::SearchTerm &subterm : subTerms) {
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
            return getTerm(term, u"organizer"_s);
        case Akonadi::IncidenceSearchTerm::Summary:
            return getTerm(term, u"summary"_s);
        case Akonadi::IncidenceSearchTerm::Location:
            return getTerm(term, u"location"_s);
        case Akonadi::IncidenceSearchTerm::PartStatus: {
            Term t(u"partstatus"_s, term.value().toString(), Term::Equal);
            t.setNegation(term.isNegated());
            return t;
        }
        default:
            if (!term.key().isEmpty()) {
                qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "unknown term " << term.key();
            }
        }
    }
    return {};
}

static Term recursiveNoteTermMapping(const Akonadi::SearchTerm &term)
{
    const auto subTerms{term.subTerms()};
    if (!subTerms.isEmpty()) {
        Term t(mapRelation(term.relation()));
        for (const Akonadi::SearchTerm &subterm : subTerms) {
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
            return getTerm(term, u"subject"_s);
        case Akonadi::EmailSearchTerm::Body:
            return getTerm(term, u"body"_s);
        default:
            if (!term.key().isEmpty()) {
                qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "unknown term " << term.key();
            }
        }
    }
    return {};
}

static Term recursiveContactTermMapping(const Akonadi::SearchTerm &term)
{
    const auto subTerms{term.subTerms()};
    if (!subTerms.isEmpty()) {
        Term t(mapRelation(term.relation()));
        for (const Akonadi::SearchTerm &subterm : subTerms) {
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
            return getTerm(term, u"name"_s);
        case Akonadi::ContactSearchTerm::Email:
            return getTerm(term, u"email"_s);
        case Akonadi::ContactSearchTerm::Nickname:
            return getTerm(term, u"nick"_s);
        case Akonadi::ContactSearchTerm::Uid:
            return getTerm(term, u"uid"_s);
        case Akonadi::ContactSearchTerm::Unknown:
        default:
            if (!term.key().isEmpty()) {
                qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "unknown term " << term.key();
            }
        }
    }
    return {};
}

QSet<qint64> SearchPlugin::search(const QString &akonadiQuery, const QList<qint64> &collections, const QStringList &mimeTypes)
{
    if (akonadiQuery.isEmpty() && collections.isEmpty() && mimeTypes.isEmpty()) {
        qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "empty query";
        return {};
    }

    Akonadi::SearchQuery searchQuery;
    if (!akonadiQuery.isEmpty()) {
        searchQuery = Akonadi::SearchQuery::fromJSON(akonadiQuery.toLatin1());
        if (searchQuery.isNull() && collections.isEmpty() && mimeTypes.isEmpty()) {
            return {};
        }
    }

    const Akonadi::SearchTerm term = searchQuery.term();

    Query query;
    Term t;

    if (mimeTypes.contains("message/rfc822"_L1)) {
        // qCDebug(AKONADIPLUGIN_INDEXER_LOG) << "mail query";
        query.setType(u"Email"_s);
        t = recursiveEmailTermMapping(term);
    } else if (mimeTypes.contains(KContacts::Addressee::mimeType()) || mimeTypes.contains(KContacts::ContactGroup::mimeType())) {
        query.setType(u"Contact"_s);
        t = recursiveContactTermMapping(term);
    } else if (mimeTypes.contains("text/x-vnd.akonadi.note"_L1)) {
        query.setType(u"Note"_s);
        t = recursiveNoteTermMapping(term);
    } else if (mimeTypes.contains("application/x-vnd.akonadi.calendar.event"_L1) || mimeTypes.contains("application/x-vnd.akonadi.calendar.todo"_L1)
               || mimeTypes.contains("application/x-vnd.akonadi.calendar.journal"_L1) || mimeTypes.contains("application/x-vnd.akonadi.calendar.freebusy"_L1)) {
        query.setType(u"Calendar"_s);
        t = recursiveCalendarTermMapping(term);
    } else {
        // Unknown type
        return {};
    }

    if (searchQuery.limit() > 0) {
        query.setLimit(searchQuery.limit());
    }

    // Filter by collection if not empty
    if (!collections.isEmpty()) {
        Term parentTerm(Term::And);
        Term collectionTerm(Term::Or);
        for (const qint64 col : collections) {
            collectionTerm.addSubTerm(Term(u"collection"_s, QString::number(col), Term::Equal));
        }
        if (t.isEmpty()) {
            query.setTerm(collectionTerm);
        } else {
            parentTerm.addSubTerm(collectionTerm);
            parentTerm.addSubTerm(t);
            query.setTerm(parentTerm);
        }
    } else {
        if (t.subTerms().isEmpty()) {
            qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "no terms added";
            return {};
        }

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

#include "moc_searchplugin.cpp"
