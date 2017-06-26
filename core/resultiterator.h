/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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
