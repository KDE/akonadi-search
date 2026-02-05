/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "search_pim_export.h"
#include <QByteArray>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
class ResultIterator;

/*!
 * \class Akonadi::Search::PIM::Query
 * \inheader AkonadiSearch/PIM/Query
 * \inmodule AkonadiSearchPIM
 * \brief Base class for PIM-specific search queries.
 *
 * Query is an abstract base class for PIM-specific search implementations.
 * Concrete implementations include EmailQuery, ContactQuery, NoteQuery,
 * and CollectionQuery.
 *
 * \sa EmailQuery, ContactQuery, NoteQuery, CollectionQuery, ResultIterator
 */
class AKONADI_SEARCH_PIM_EXPORT Query
{
public:
    /*!
     * \brief Constructs a query.
     */
    Query();
    /*!
     * \brief Destructs the query.
     */
    virtual ~Query();
    /*!
     * \brief Executes the query and returns results.
     * \return An iterator over the search results.
     */
    virtual ResultIterator exec() = 0;

    /*!
     * \brief Creates a query from a JSON representation.
     * \param json The JSON representation of a query.
     * \return A new query object, or nullptr on error.
     */
    static Query *fromJSON(const QByteArray &json);
    /*!
     * \brief Returns the default location for a search database.
     * \param dbName The database name.
     * \return The default file path for the database.
     */
    [[nodiscard]] static QString defaultLocation(const QString &dbName);
};
}
}
}
