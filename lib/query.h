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
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef AKONADI_SEARCH_PIM_QUERY_H
#define AKONADI_SEARCH_PIM_QUERY_H

#include "search_pim_export.h"
#include <QByteArray>

namespace Akonadi
{
namespace Search
{
namespace PIM
{

class ResultIterator;

/** Query base class. */
class AKONADI_SEARCH_PIM_EXPORT Query
{
public:
    Query();
    virtual ~Query();
    virtual ResultIterator exec() = 0;

    static Query *fromJSON(const QByteArray &json);
    static QString defaultLocation(const QString &dbName);
};

}
}
}

#endif // AKONADI_SEARCH_PIM_QUERY_H
