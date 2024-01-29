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

Term recursiveEmailTermMapping(const Akonadi::SearchTerm &term)
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
            Term s(QStringLiteral("date"), QString::number(term.value().toDateTime().toSecsSinceEpoch()), mapComparator(term.condition()));
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
        case Akonadi::EmailSearchTerm::MessageStatus: {
            const QString value = term.value().toString();
            if (value == QLatin1StringView(Akonadi::MessageFlags::Flagged)) {
                return Term(QStringLiteral("isimportant"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::ToAct)) {
                return Term(QStringLiteral("istoact"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Watched)) {
                return Term(QStringLiteral("iswatched"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Deleted)) {
                return Term(QStringLiteral("isdeleted"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Spam)) {
                return Term(QStringLiteral("isspam"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Replied)) {
                return Term(QStringLiteral("isreplied"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Ignored)) {
                return Term(QStringLiteral("isignored"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Forwarded)) {
                return Term(QStringLiteral("isforwarded"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Sent)) {
                return Term(QStringLiteral("issent"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Queued)) {
                return Term(QStringLiteral("isqueued"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Ham)) {
                return Term(QStringLiteral("isham"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Seen)) {
                return Term(QStringLiteral("isread"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::HasAttachment)) {
                return Term(QStringLiteral("hasattachment"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::Encrypted)) {
                return Term(QStringLiteral("isencrypted"), !term.isNegated());
            }
            if (value == QLatin1StringView(Akonadi::MessageFlags::HasInvitation)) {
                return Term(QStringLiteral("hasinvitation"), !term.isNegated());
            }
            break;
        }
        case Akonadi::EmailSearchTerm::MessageTag:
            // search directly in akonadi? or index tags.
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
        case Akonadi::EmailSearchTerm::Attachment:
            return Term(QStringLiteral("hasattachment"), !term.isNegated());
        case Akonadi::EmailSearchTerm::Unknown:
        default:
            if (!term.key().isEmpty()) {
                qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "unknown term " << term.key();
            }
        }
    }
    return {};
}

Term recursiveCalendarTermMapping(const Akonadi::SearchTerm &term)
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
            if (!term.key().isEmpty()) {
                qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "unknown term " << term.key();
            }
        }
    }
    return {};
}

Term recursiveNoteTermMapping(const Akonadi::SearchTerm &term)
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
            return getTerm(term, QStringLiteral("subject"));
        case Akonadi::EmailSearchTerm::Body:
            return getTerm(term, QStringLiteral("body"));
        default:
            if (!term.key().isEmpty()) {
                qCWarning(AKONADIPLUGIN_INDEXER_LOG) << "unknown term " << term.key();
            }
        }
    }
    return {};
}

Term recursiveContactTermMapping(const Akonadi::SearchTerm &term)
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
            return getTerm(term, QStringLiteral("name"));
        case Akonadi::ContactSearchTerm::Email:
            return getTerm(term, QStringLiteral("email"));
        case Akonadi::ContactSearchTerm::Nickname:
            return getTerm(term, QStringLiteral("nick"));
        case Akonadi::ContactSearchTerm::Uid:
            return getTerm(term, QStringLiteral("uid"));
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

    if (mimeTypes.contains(QLatin1StringView("message/rfc822"))) {
        // qCDebug(AKONADIPLUGIN_INDEXER_LOG) << "mail query";
        query.setType(QStringLiteral("Email"));
        t = recursiveEmailTermMapping(term);
    } else if (mimeTypes.contains(KContacts::Addressee::mimeType()) || mimeTypes.contains(KContacts::ContactGroup::mimeType())) {
        query.setType(QStringLiteral("Contact"));
        t = recursiveContactTermMapping(term);
    } else if (mimeTypes.contains(QLatin1StringView("text/x-vnd.akonadi.note"))) {
        query.setType(QStringLiteral("Note"));
        t = recursiveNoteTermMapping(term);
    } else if (mimeTypes.contains(QLatin1StringView("application/x-vnd.akonadi.calendar.event"))
               || mimeTypes.contains(QLatin1StringView("application/x-vnd.akonadi.calendar.todo"))
               || mimeTypes.contains(QLatin1StringView("application/x-vnd.akonadi.calendar.journal"))
               || mimeTypes.contains(QLatin1StringView("application/x-vnd.akonadi.calendar.freebusy"))) {
        query.setType(QStringLiteral("Calendar"));
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
            collectionTerm.addSubTerm(Term(QStringLiteral("collection"), QString::number(col), Term::Equal));
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
