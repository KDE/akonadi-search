/*
 * This file is part of the KDE Baloo Project
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
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _COLLECTION_QUERY_H
#define _COLLECTION_QUERY_H

#include "pim_export.h"
#include "query.h"
#include "resultiterator.h"

#include <QStringList>
#include <AkonadiCore/Collection>

namespace Baloo
{
namespace PIM
{

class BALOO_PIM_EXPORT CollectionQuery : public Query
{
public:
    CollectionQuery();
    virtual ~CollectionQuery();

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
    ResultIterator exec();

    /**
     * For testing
     */
    void setDatabaseDir(const QString &dir);

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}
}

#endif // _COLLECTION_QUERY_H
