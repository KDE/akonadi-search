/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2022 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "../pimsearchstore.h"

namespace Akonadi
{
namespace Search
{
class NoteSearchStore : public PIMSearchStore
{
    Q_OBJECT
    Q_INTERFACES(Akonadi::Search::SearchStore)
#ifndef AKONADI_SEARCH_NO_PLUGINS
    Q_PLUGIN_METADATA(IID "org.kde.Akonadi.Search.SearchStore" FILE "notesearchstore.json")
#endif
public:
    explicit NoteSearchStore(QObject *parent = nullptr);

    QStringList types() override;
};
}
}
