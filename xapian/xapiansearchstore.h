/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include <xapian.h>

#include "core/searchstore.h"
#include "core/term.h"
#include "search_xapian_export.h"

#include <QMutex>

namespace Akonadi
{
namespace Search
{
/**
 * Implements a search store using Xapian
 */
class AKONADI_SEARCH_XAPIAN_EXPORT XapianSearchStore : public SearchStore
{
    Q_OBJECT
public:
    explicit XapianSearchStore(QObject *parent = nullptr);
    ~XapianSearchStore() override;

    int exec(const Query &query) override;
    void close(int queryId) override;
    bool next(int queryId) override;

    Q_REQUIRED_RESULT QByteArray id(int queryId) override;
    Q_REQUIRED_RESULT QUrl url(int queryId) override;

    /**
     * Set the path of the xapian database
     */
    virtual void setDbPath(const QString &path);
    virtual QString dbPath();

protected:
    /**
     * The derived class should implement the logic for constructing the appropriate
     * Xapian::Query class from the given values.
     */
    virtual Xapian::Query constructQuery(const QString &property, const QVariant &value, Term::Comparator com) = 0;

    virtual Xapian::Query constructFilterQuery(int year, int month, int day);

    /**
     * Apply any final touches to the query
     */
    virtual Xapian::Query finalizeQuery(const Xapian::Query &query);

    /**
     * Create a query for any custom options.
     */
    virtual Xapian::Query applyCustomOptions(const Xapian::Query &q, const QVariantMap &options);

    /**
     * Returns the url for the document with id \p docid.
     */
    virtual QUrl constructUrl(const Xapian::docid &docid) = 0;

    /**
     * Gives a list of types which have been provided with the query.
     * This must return the appropriate query which will be ANDed with
     * the final query
     */
    virtual Xapian::Query convertTypes(const QStringList &types) = 0;

    /**
     * The prefix that should be used when converting an integer
     * id to a byte array
     */
    virtual QByteArray idPrefix() = 0;

    Xapian::Document docForQuery(int queryId);

    /**
     * Convenience function to AND two Xapian queries together.
     */
    Xapian::Query andQuery(const Xapian::Query &a, const Xapian::Query &b);

    Xapian::Database *xapianDb();

protected:
    QRecursiveMutex m_mutex;

private:
    Xapian::Query toXapianQuery(const Term &term);
    Xapian::Query toXapianQuery(Xapian::Query::op op, const QList<Term> &terms);

    Xapian::Query constructSearchQuery(const QString &str);

    struct Result {
        Xapian::MSet mset;
        Xapian::MSetIterator it;

        uint lastId;
        QUrl lastUrl;
    };

    QHash<int, Result> m_queryMap;
    int m_nextId = 1;

    QString m_dbPath;

    Xapian::Database *m_db = nullptr;
};
}
}
