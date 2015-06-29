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
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "contactsearchstore.h"

using namespace Akonadi::Search;

ContactSearchStore::ContactSearchStore(QObject *parent)
    : PIMSearchStore(parent)
{
    m_prefix.insert(QStringLiteral("name"), QStringLiteral("NA"));
    m_prefix.insert(QStringLiteral("nick"), QStringLiteral("NI"));
    m_prefix.insert(QStringLiteral("email"), QStringLiteral("")); // Email currently doesn't map to anything
    m_prefix.insert(QStringLiteral("collection"), QStringLiteral("C"));

    m_valueProperties.insert(QStringLiteral("birthday"), 0);
    m_valueProperties.insert(QStringLiteral("anniversary"), 1);

    setDbPath(findDatabase(QStringLiteral("contacts")));
}

QStringList ContactSearchStore::types()
{
    return QStringList() << QStringLiteral("Akonadi") << QStringLiteral("Contact");
}

