/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2025 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "notesearchstore.h"
using namespace Qt::Literals::StringLiterals;

using namespace Akonadi::Search;

NoteSearchStore::NoteSearchStore(QObject *parent)
    : PIMSearchStore(parent)
{
    m_prefix.insert(u"subject"_s, u"SU"_s);
    m_prefix.insert(u"collection"_s, u"C"_s);
    m_prefix.insert(u"body"_s, u"BO"_s);

    setDbPath(findDatabase(u"notes"_s));
}

QStringList NoteSearchStore::types()
{
    return QStringList() << u"Akonadi"_s << u"Note"_s;
}

#include "moc_notesearchstore.cpp"
