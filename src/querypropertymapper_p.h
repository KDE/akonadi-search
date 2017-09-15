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
    explicit QueryPropertyMapper();

    void insertPrefix(const QString &property, const QString &prefix);
    void insertBoolProperty(const QString &property);
    void insertBoolValueProperty(const QString &property);
    void insertValueProperty(const QString &property, int value);

    bool hasPrefix(const QString &prop) const;
    bool hasBoolProperty(const QString &prop) const;
    bool hasBoolValueProperty(const QString &prop) const;
    bool hasValueProperty(const QString &prop) const;

    const std::string &prefix(const QString &prop) const;
    int valueProperty(const QString &prop) const;

private:
    QSet<QString> mBoolProperties;
    QSet<QString> mBoolValueProperties;
    QHash<QString, std::string> mPrefixes;
    QHash<QString, int> mValueProperties;
};

}
}

#endif
