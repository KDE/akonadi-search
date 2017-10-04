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

using namespace Akonadi::Search;

QueryPropertyMapper::QueryPropertyMapper()
{
    insertPrefix(QStringLiteral("collection"), QStringLiteral("C"));
    insertBoolValueProperty(QStringLiteral("collection"));
}

void QueryPropertyMapper::insertBoolProperty(const QString &prop)
{
    mBoolProperties.insert(prop);
}

void QueryPropertyMapper::insertPrefix(const QString &prop, const QString &prefix)
{
    mPrefixes.insert(prop, prefix.toStdString());
}

void QueryPropertyMapper::insertValueProperty(const QString &prop, int value)
{
    mValueProperties.insert(prop, value);
}

void QueryPropertyMapper::insertBoolValueProperty(const QString &prop)
{
    mBoolValueProperties.insert(prop);
}

bool QueryPropertyMapper::hasBoolProperty(const QString &prop) const
{
    return mBoolProperties.contains(prop);
}

bool QueryPropertyMapper::hasPrefix(const QString &prop) const
{
    return mPrefixes.contains(prop);
}

bool QueryPropertyMapper::hasValueProperty(const QString &prop) const
{
    return mValueProperties.contains(prop);
}

bool QueryPropertyMapper::hasBoolValueProperty(const QString &prop) const
{
    return mBoolValueProperties.contains(prop);
}

int QueryPropertyMapper::valueProperty(const QString &prop) const
{
    return mValueProperties.value(prop);
}

const std::string &QueryPropertyMapper::prefix(const QString &prop) const
{
    Q_ASSERT(hasPrefix(prop));
    return *mPrefixes.constFind(prop);
}
