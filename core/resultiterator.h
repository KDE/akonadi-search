/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

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
    {
    }

    ~ResultIteratorPrivate()
    {
        if (store) {
            store->close(queryId);
        }
    }

    int queryId = 0;
    SearchStore *store = nullptr;
};

/** Result iterator. */
class AKONADI_SEARCH_CORE_EXPORT ResultIterator
{
public:
    /*!
     */
    ResultIterator();
    /*!
     */
    ResultIterator(const ResultIterator &rhs);
    /*!
     */
    ~ResultIterator();

    /*!
     */
    ResultIterator &operator=(const ResultIterator &other);

    // internal
    ResultIterator(int id, SearchStore *store);

    /*!
     */
    bool next();

    /*!
     */
    [[nodiscard]] QByteArray id() const;
    /*!
     */
    [[nodiscard]] QUrl url() const;

    /*!
     */
    [[nodiscard]] QString text() const;
    /*!
     */
    [[nodiscard]] QString icon() const;

private:
    QExplicitlySharedDataPointer<ResultIteratorPrivate> d;
};
}
}
