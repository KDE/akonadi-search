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

#ifndef AKONADISEARCH_QUERYPROPERTYMAPPER_P_H_
#define AKONADISEARCH_QUERYPROPERTYMAPPER_P_H_

#include <QString>
#include <QSet>

namespace Akonadi {
namespace Search {

class QueryPropertyMapper
{
public:
    void insertPrefix(int propertyKey, const QString &prefix);
    void insertBoolProperty(int propertyKey);
    void insertBoolValueProperty(int propertyKey);
    void insertValueProperty(int propertyKey, int value);

    bool hasPrefix(int propertyKey) const;
    bool hasBoolProperty(int propertyKey) const;
    bool hasBoolValueProperty(int propertyKey) const;
    bool hasValueProperty(int propertyKey) const;

    const std::string &prefix(int propertyKey) const;
    int valueProperty(int propertyKey) const;

protected:
    explicit QueryPropertyMapper();

private:
    QSet<int> mBoolProperties;
    QSet<int> mBoolValueProperties;
    QHash<int, std::string> mPrefixes;
    QHash<int, int> mValueProperties;

    Q_DISABLE_COPY(QueryPropertyMapper)
};

}
}

#endif
