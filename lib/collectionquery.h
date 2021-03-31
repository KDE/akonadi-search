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

#include <AkonadiCore/Collection>
#include <QStringList>

namespace Akonadi
{
namespace Search
{
/** PIM specific search API. */
namespace PIM
{
/** Collection query. */
class AKONADI_SEARCH_PIM_EXPORT CollectionQuery : public Query
{
public:
    CollectionQuery();
    ~CollectionQuery() override;

    void setNamespace(const QStringList &ns);
    void setMimetype(const QStringList &mt);

    /**
     * Matches the string \p match in the name.
     */
    void nameMatches(const QString &match);
    void identifierMatches(const QString &match);
    void pathMatches(const QString &match);

    void setLimit(int limit);
    int limit() const;

    /**
     * Execute the query and return an iterator to fetch
     * the results
     */
    ResultIterator exec() override;

    /**
     * For testing
     */
    void setDatabaseDir(const QString &dir);

private:
    //@cond PRIVATE
    struct Private;
    Private *const d;
    //@endcond
};
}
}
}

