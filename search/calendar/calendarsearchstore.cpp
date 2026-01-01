/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2026 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "calendarsearchstore.h"
using namespace Qt::Literals::StringLiterals;

using namespace Akonadi::Search;

CalendarSearchStore::CalendarSearchStore(QObject *parent)
    : PIMSearchStore(parent)
{
    m_prefix.insert(u"collection"_s, u"C"_s);
    m_prefix.insert(u"organizer"_s, u"O"_s);
    m_prefix.insert(u"partstatus"_s, u"PS"_s);
    m_prefix.insert(u"summary"_s, u"S"_s);
    m_prefix.insert(u"location"_s, u"L"_s);

    m_boolWithValue << u"partstatus"_s;

    setDbPath(findDatabase(u"calendars"_s));
}

QStringList CalendarSearchStore::types()
{
    return QStringList() << u"Akonadi"_s << u"Calendar"_s;
}

#include "moc_calendarsearchstore.cpp"
