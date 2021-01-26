/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#ifndef AKONADI_SEARCH_CORE_RESULT_ITERATOR_H
#define AKONADI_SEARCH_CORE_RESULT_ITERATOR_H

#include "search_core_export.h"
#include "searchstore.h"

#include <QExplicitlySharedDataPointer>

namespace Akonadi
{
namespace Search
{
class SearchStore;
class Result;

class Q_DECL_HIDDEN ResultIteratorPrivate : public QSharedData
{
public:
    ResultIteratorPrivate()
        : queryId(0)
        , store(nullptr)
    {
    }

    ~ResultIteratorPrivate()
    {
        if (store) {
            store->close(queryId);
        }
    }

    int queryId;
    SearchStore *store;
};

/** Result iterator. */
class AKONADI_SEARCH_CORE_EXPORT ResultIterator
{
public:
    ResultIterator();
    ResultIterator(const ResultIterator &rhs);
    ~ResultIterator();

    ResultIterator &operator=(const ResultIterator &other);

    // internal
    ResultIterator(int id, SearchStore *store);

    bool next();

    QByteArray id() const;
    QUrl url() const;

    QString text() const;
    QString icon() const;

private:
    QExplicitlySharedDataPointer<ResultIteratorPrivate> d;
};
}
}

#endif // AKONADI_SEARCH_CORE_RESULT_ITERATOR_H
