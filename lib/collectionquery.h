/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "query.h"
#include "resultiterator.h"
#include "search_pim_export.h"

#include <Akonadi/Collection>
#include <QStringList>

#include <memory>

namespace Akonadi
{
namespace Search
{
/*! PIM specific search API. */
namespace PIM
{
class CollectionQueryPrivate;

/*!
 * \class Akonadi::Search::PIM::CollectionQuery
 * \inheader AkonadiSearch/PIM/CollectionQuery
 * \inmodule AkonadiSearchPIM
 * \brief Search query for Akonadi collections.
 *
 * CollectionQuery allows searching for Akonadi collections by name,
 * identifier, path, namespace, and MIME type filters.
 *
 * \sa Query, ResultIterator
 */
class AKONADI_SEARCH_PIM_EXPORT CollectionQuery : public Query
{
public:
    /*!
     * \brief Constructs an empty collection query.
     */
    CollectionQuery();
    /*!
     * \brief Destructs the collection query.
     */
    ~CollectionQuery() override;

    /*!
     * \brief Sets the namespace filter for the query.
     * \param ns The list of namespaces to search in.
     */
    void setNamespace(const QStringList &ns);
    /*!
     * \brief Sets the MIME type filter for the query.
     * \param mt The list of MIME types to search for.
     */
    void setMimetype(const QStringList &mt);

    /*!
     * \brief Filters collections by name.
     * \param match The string to match in the collection name.
     */
    void nameMatches(const QString &match);
    /*!
     * \brief Filters collections by identifier.
     * \param match The string to match in the identifier.
     */
    void identifierMatches(const QString &match);
    /*!
     * \brief Filters collections by path.
     * \param match The string to match in the path.
     */
    void pathMatches(const QString &match);

    /*!
     * \brief Sets the maximum number of results to return.
     * \param limit The result limit.
     */
    void setLimit(int limit);
    /*!
     * \brief Returns the maximum number of results for this query.
     * \return The result limit.
     */
    [[nodiscard]] int limit() const;

    /*!
     * \brief Executes the query and returns an iterator to fetch results.
     * \return An iterator over the query results.
     */
    [[nodiscard]] ResultIterator exec() override;

    /*!
     * For testing
     */
    void setDatabaseDir(const QString &dir);

private:
    std::unique_ptr<CollectionQueryPrivate> const d;
};
}
}
}
