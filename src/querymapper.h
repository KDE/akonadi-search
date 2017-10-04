/*
 * Copyright (C) 2017  Daniel Vrátil <dvratil@kde.org>
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

#ifndef AKONADISEARCH_QUERYMAPPER_H_
#define AKONADISEARCH_QUERYMAPPER_H_

#include <akonadisearch_export.h>

#include <QVector>

class QString;

namespace Xapian {
class Query;
}

namespace Akonadi {

class SearchQuery;
class SearchTerm;

namespace Search {

class QueryPropertyMapper;

class AKONADISEARCH_EXPORT QueryMapper
{
public:
    static QVector<QueryMapper*> forType(const QString &mimeType);

    virtual ~QueryMapper();

    Xapian::Query map(const Akonadi::SearchQuery &akQuery);

protected:
    virtual Xapian::Query recursiveTermMapping(const Akonadi::SearchTerm &term);
    virtual QueryPropertyMapper &propertyMapper() = 0;

    explicit QueryMapper();
};

}
}


#endif