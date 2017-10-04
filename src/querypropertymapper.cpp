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

#include "querypropertymapper_p.h"

#include <AkonadiCore/SearchQuery>

using namespace Akonadi::Search;

QueryPropertyMapper::QueryPropertyMapper()
{
    insertPrefix(Akonadi::SearchTerm::Collection, QStringLiteral("C"));
    insertBoolValueProperty(Akonadi::SearchTerm::Collection);
}

void QueryPropertyMapper::insertBoolProperty(int propertyKey)
{
    mBoolProperties.insert(propertyKey);
}

void QueryPropertyMapper::insertPrefix(int propertyKey, const QString &prefix)
{
    mPrefixes.insert(propertyKey, prefix.toStdString());
}

void QueryPropertyMapper::insertValueProperty(int propertyKey, int value)
{
    mValueProperties.insert(propertyKey, value);
}

void QueryPropertyMapper::insertBoolValueProperty(int propertyKey)
{
    mBoolValueProperties.insert(propertyKey);
}

bool QueryPropertyMapper::hasBoolProperty(int propertyKey) const
{
    return mBoolProperties.contains(propertyKey);
}

bool QueryPropertyMapper::hasPrefix(int propertyKey) const
{
    return mPrefixes.contains(propertyKey);
}

bool QueryPropertyMapper::hasValueProperty(int propertyKey) const
{
    return mValueProperties.contains(propertyKey);
}

bool QueryPropertyMapper::hasBoolValueProperty(int propertyKey) const
{
    return mBoolValueProperties.contains(propertyKey);
}

int QueryPropertyMapper::valueProperty(int propertyKey) const
{
    return mValueProperties.value(propertyKey);
}

const std::string &QueryPropertyMapper::prefix(int propertyKey) const
{
    Q_ASSERT(hasPrefix(propertyKey));
    return *mPrefixes.constFind(propertyKey);
}
