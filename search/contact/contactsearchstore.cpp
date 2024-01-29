/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "contactsearchstore.h"

using namespace Akonadi::Search;

ContactSearchStore::ContactSearchStore(QObject *parent)
    : PIMSearchStore(parent)
{
    m_prefix.insert(QStringLiteral("name"), QStringLiteral("NA"));
    m_prefix.insert(QStringLiteral("nick"), QStringLiteral("NI"));
    m_prefix.insert(QStringLiteral("email"), QLatin1StringView("")); // Email currently doesn't map to anything
    m_prefix.insert(QStringLiteral("collection"), QStringLiteral("C"));
    m_prefix.insert(QStringLiteral("uid"), QStringLiteral("UID"));

    m_valueProperties.insert(QStringLiteral("birthday"), 0);
    m_valueProperties.insert(QStringLiteral("anniversary"), 1);

    setDbPath(findDatabase(QStringLiteral("contacts")));
}

QStringList ContactSearchStore::types()
{
    return QStringList() << QStringLiteral("Akonadi") << QStringLiteral("Contact");
}

#include "moc_contactsearchstore.cpp"
