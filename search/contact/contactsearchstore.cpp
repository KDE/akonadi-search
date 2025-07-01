/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "contactsearchstore.h"

using namespace Qt::Literals::StringLiterals;
using namespace Akonadi::Search;

ContactSearchStore::ContactSearchStore(QObject *parent)
    : PIMSearchStore(parent)
{
    m_prefix.insert(u"name"_s, u"NA"_s);
    m_prefix.insert(u"nick"_s, u"NI"_s);
    m_prefix.insert(u"email"_s, ""_L1); // Email currently doesn't map to anything
    m_prefix.insert(u"collection"_s, u"C"_s);
    m_prefix.insert(u"uid"_s, u"UID"_s);

    m_valueProperties.insert(u"birthday"_s, 0);
    m_valueProperties.insert(u"anniversary"_s, 1);

    setDbPath(findDatabase(u"contacts"_s));
}

QStringList ContactSearchStore::types()
{
    return QStringList() << u"Akonadi"_s << u"Contact"_s;
}

#include "moc_contactsearchstore.cpp"
