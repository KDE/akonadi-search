/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2021 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#ifndef AKONADI_SEARCH_PIM_CALENDAR_SEARCHSTORE_H
#define AKONADI_SEARCH_PIM_CALENDAR_SEARCHSTORE_H

#include "../pimsearchstore.h"

namespace Akonadi
{
namespace Search
{
class CalendarSearchStore : public PIMSearchStore
{
    Q_OBJECT
    Q_INTERFACES(Akonadi::Search::SearchStore)
#ifndef AKONADI_SEARCH_NO_PLUGINS
    Q_PLUGIN_METADATA(IID "org.kde.Akonadi.Search.SearchStore" FILE "calendarsearchstore.json")
#endif
public:
    explicit CalendarSearchStore(QObject *parent = nullptr);

    QStringList types() override;
};
}
}
#endif // AKONADI_SEARCH_PIM_CALENDAR_SEARCHSTORE_H
