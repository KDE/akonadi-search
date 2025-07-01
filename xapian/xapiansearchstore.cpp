/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013-2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "xapiansearchstore.h"
using namespace Qt::Literals::StringLiterals;

#include "query.h"
#include "xapianqueryparser.h"

#include "akonadi_search_xapian_debug.h"
#include <QList>

#include <algorithm>

using namespace Akonadi::Search;

XapianSearchStore::XapianSearchStore(QObject *parent)
    : SearchStore(parent)

{
}

XapianSearchStore::~XapianSearchStore()
{
    delete m_db;
}

void XapianSearchStore::setDbPath(const QString &path)
{
    m_dbPath = path;

    delete m_db;
    m_db = nullptr;

    try {
        m_db = new Xapian::Database(m_dbPath.toStdString());
    } catch (const Xapian::DatabaseOpeningError &) {
        qCWarning(AKONADI_SEARCH_XAPIAN_LOG) << "Xapian Database does not exist at " << m_dbPath;
    } catch (const Xapian::DatabaseCorruptError &) {
        qCWarning(AKONADI_SEARCH_XAPIAN_LOG) << "Xapian Database corrupted at " << m_dbPath;
    } catch (...) {
        qCWarning(AKONADI_SEARCH_XAPIAN_LOG) << "Random exception, but we do not want to crash";
    }
}

QString XapianSearchStore::dbPath()
{
    return m_dbPath;
}

Xapian::Query XapianSearchStore::toXapianQuery(Xapian::Query::op op, const QList<Term> &terms)
{
    Q_ASSERT_X(op == Xapian::Query::OP_AND || op == Xapian::Query::OP_OR, "XapianSearchStore::toXapianQuery", "The op must be AND / OR");

    QList<Xapian::Query> queries;
    queries.reserve(terms.size());

    for (const Term &term : terms) {
        const Xapian::Query q = toXapianQuery(term);
        queries << q;
    }

    return {op, queries.begin(), queries.end()};
}

static Xapian::Query negate(bool shouldNegate, const Xapian::Query &query)
{
    if (shouldNegate) {
        return Xapian::Query(Xapian::Query::OP_AND_NOT, Xapian::Query::MatchAll, query);
    }
    return query;
}

Xapian::Query XapianSearchStore::toXapianQuery(const Term &term)
{
    if (term.operation() == Term::And) {
        return negate(term.isNegated(), toXapianQuery(Xapian::Query::OP_AND, term.subTerms()));
    }
    if (term.operation() == Term::Or) {
        return negate(term.isNegated(), toXapianQuery(Xapian::Query::OP_OR, term.subTerms()));
    }

    return negate(term.isNegated(), constructQuery(term.property(), term.value(), term.comparator()));
}

Xapian::Query XapianSearchStore::andQuery(const Xapian::Query &a, const Xapian::Query &b)
{
    if (a.empty() && !b.empty()) {
        return b;
    }
    if (!a.empty() && b.empty()) {
        return a;
    }
    if (a.empty() && b.empty()) {
        return {};
    } else {
        return Xapian::Query(Xapian::Query::OP_AND, a, b);
    }
}

Xapian::Query XapianSearchStore::constructSearchQuery(const QString &str)
{
    XapianQueryParser parser;
    parser.setDatabase(m_db);
    return parser.parseQuery(str);
}

int XapianSearchStore::exec(const Query &query)
{
    if (!m_db) {
        return 0;
    }

    while (true) {
        try {
            QMutexLocker lock(&m_mutex);
            try {
                m_db->reopen();
            } catch (Xapian::DatabaseError &e) {
                qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << "Failed to reopen database" << dbPath() << ":" << QString::fromStdString(e.get_msg());
                return 0;
            }

            Xapian::Query xapQ = toXapianQuery(query.term());
            // The term was not properly converted. Lets abort. The properties
            // must not exist
            if (!query.term().empty() && xapQ.empty()) {
                qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << query.term() << "could not be processed. Aborting";
                return 0;
            }
            if (!query.searchString().isEmpty()) {
                QString str = query.searchString();

                Xapian::Query q = constructSearchQuery(str);
                xapQ = andQuery(xapQ, q);
            }
            xapQ = andQuery(xapQ, convertTypes(query.types()));
            xapQ = andQuery(xapQ, constructFilterQuery(query.yearFilter(), query.monthFilter(), query.dayFilter()));
            xapQ = applyCustomOptions(xapQ, query.customOptions());
            xapQ = finalizeQuery(xapQ);

            if (xapQ.empty()) {
                // Return all the results
                xapQ = Xapian::Query(std::string());
            }
            Xapian::Enquire enquire(*m_db);
            enquire.set_query(xapQ);

            if (query.sortingOption() == Query::SortNone) {
                enquire.set_weighting_scheme(Xapian::BoolWeight());
            }

            Result &res = m_queryMap[m_nextId++];
            res.mset = enquire.get_mset(query.offset(), query.limit());
            res.it = res.mset.begin();

            return m_nextId - 1;
        } catch (const Xapian::DatabaseModifiedError &) {
            continue;
        } catch (const Xapian::Error &) {
            return 0;
        }
    }

    return 0;
}

void XapianSearchStore::close(int queryId)
{
    QMutexLocker lock(&m_mutex);
    m_queryMap.remove(queryId);
}

QByteArray XapianSearchStore::id(int queryId)
{
    QMutexLocker lock(&m_mutex);
    Q_ASSERT_X(m_queryMap.contains(queryId), "FileSearchStore::id", "Passed a queryId which does not exist");

    const Result res = m_queryMap.value(queryId);
    if (!res.lastId) {
        return {};
    }

    return serialize(idPrefix(), res.lastId);
}

QUrl XapianSearchStore::url(int queryId)
{
    QMutexLocker lock(&m_mutex);
    Result &res = m_queryMap[queryId];

    if (!res.lastId) {
        return {};
    }

    if (!res.lastUrl.isEmpty()) {
        return res.lastUrl;
    }

    res.lastUrl = constructUrl(res.lastId);
    return res.lastUrl;
}

bool XapianSearchStore::next(int queryId)
{
    if (!m_db) {
        return false;
    }

    QMutexLocker lock(&m_mutex);
    Result &res = m_queryMap[queryId];

    bool atEnd = (res.it == res.mset.end());
    if (atEnd) {
        res.lastId = 0;
        res.lastUrl.clear();
    } else {
        res.lastId = *res.it;
        res.lastUrl.clear();
        ++res.it;
    }

    return !atEnd;
}

Xapian::Document XapianSearchStore::docForQuery(int queryId)
{
    if (!m_db) {
        return {};
    }

    QMutexLocker lock(&m_mutex);

    try {
        const Result &res = m_queryMap.value(queryId);
        if (!res.lastId) {
            return {};
        }

        return m_db->get_document(res.lastId);
    } catch (const Xapian::DocNotFoundError &) {
        return {};
    } catch (const Xapian::DatabaseModifiedError &) {
        m_db->reopen();
        return docForQuery(queryId);
    } catch (const Xapian::Error &) {
        return {};
    }
}

Xapian::Database *XapianSearchStore::xapianDb()
{
    return m_db;
}

Xapian::Query XapianSearchStore::constructFilterQuery(int year, int month, int day)
{
    Q_UNUSED(year)
    Q_UNUSED(month)
    Q_UNUSED(day)
    return {};
}

Xapian::Query XapianSearchStore::finalizeQuery(const Xapian::Query &query)
{
    return query;
}

Xapian::Query XapianSearchStore::applyCustomOptions(const Xapian::Query &q, const QVariantMap &options)
{
    Q_UNUSED(options)
    return q;
}

#include "moc_xapiansearchstore.cpp"
