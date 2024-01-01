/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2024 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "calendarsearchstore.h"

using namespace Akonadi::Search;

CalendarSearchStore::CalendarSearchStore(QObject *parent)
    : PIMSearchStore(parent)
{
    m_prefix.insert(QStringLiteral("collection"), QStringLiteral("C"));
    m_prefix.insert(QStringLiteral("organizer"), QStringLiteral("O"));
    m_prefix.insert(QStringLiteral("partstatus"), QStringLiteral("PS"));
    m_prefix.insert(QStringLiteral("summary"), QStringLiteral("S"));
    m_prefix.insert(QStringLiteral("location"), QStringLiteral("L"));

    m_boolWithValue << QStringLiteral("partstatus");

    setDbPath(findDatabase(QStringLiteral("calendars")));
}

QStringList CalendarSearchStore::types()
{
    return QStringList() << QStringLiteral("Akonadi") << QStringLiteral("Calendar");
}

#include "moc_calendarsearchstore.cpp"
