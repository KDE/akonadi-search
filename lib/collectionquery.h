/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2014  Christian Mollekopf <mollekopf@kolabsys.com>
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
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef AKONADI_SEARCH_PIM_COLLECTION_QUERY_H
#define AKONADI_SEARCH_PIM_COLLECTION_QUERY_H

#include "search_pim_export.h"
#include "query.h"
#include "resultiterator.h"

#include <QStringList>
#include <AkonadiCore/Collection>

namespace Akonadi {
namespace Search {
/** PIM specific search API. */
namespace PIM {
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

#endif // AKONADI_SEARCH_PIM_COLLECTION_QUERY_H
