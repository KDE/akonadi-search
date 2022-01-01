/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2022 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "notesearchstore.h"

using namespace Akonadi::Search;

NoteSearchStore::NoteSearchStore(QObject *parent)
    : PIMSearchStore(parent)
{
    m_prefix.insert(QStringLiteral("subject"), QStringLiteral("SU"));
    m_prefix.insert(QStringLiteral("collection"), QStringLiteral("C"));
    m_prefix.insert(QStringLiteral("body"), QStringLiteral("BO"));

    setDbPath(findDatabase(QStringLiteral("notes")));
}

QStringList NoteSearchStore::types()
{
    return QStringList() << QStringLiteral("Akonadi") << QStringLiteral("Note");
}
