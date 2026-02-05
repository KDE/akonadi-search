/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "search_pim_export.h"

#include <Akonadi/Item>

#include <memory>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
class ContactQuery;
class EmailQuery;
class NoteQuery;
class ResultIteratorPrivate;

/*!
 * \class Akonadi::Search::PIM::ResultIterator
 * \inheader AkonadiSearch/PIM/ResultIterator
 * \inmodule AkonadiSearchPIM
 * \brief Iterator for traversing search results.
 *
 * ResultIterator provides a way to iterate through results returned by
 * PIM search queries (EmailQuery, ContactQuery, NoteQuery, CollectionQuery).
 *
 * \sa EmailQuery, ContactQuery, NoteQuery, CollectionQuery
 */
class AKONADI_SEARCH_PIM_EXPORT ResultIterator
{
public:
    /*!
     * \brief Constructs an empty result iterator.
     */
    ResultIterator();
    /*!
     * \brief Constructs a copy of the result iterator \a ri.
     * \param ri The iterator to copy.
     */
    ResultIterator(const ResultIterator &ri);
    /*!
     * \brief Destructs the result iterator.
     */
    ~ResultIterator();

    /*!
     * \brief Returns the ID of the current result item.
     * \return The item ID.
     */
    [[nodiscard]] Akonadi::Item::Id id();
    /*!
     * \brief Advances to the next result.
     * \return \c true if there is a next result, \c false if no more results.
     */
    bool next();

private:
    friend class ContactQuery;
    friend class EmailQuery;
    friend class NoteQuery;
    friend class CollectionQuery;

    std::unique_ptr<ResultIteratorPrivate> const d;
};
}
}
}
