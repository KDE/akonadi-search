/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2012  Vishesh Handa <me@vhanda.in>
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
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef AKONADI_SEARCH_PIM_CONTACT_SEARCHSTORE_H
#define AKONADI_SEARCH_PIM_CONTACT_SEARCHSTORE_H

#include "../pimsearchstore.h"

namespace Akonadi {
namespace Search {
class ContactSearchStore : public PIMSearchStore
{
    Q_OBJECT
    Q_INTERFACES(Akonadi::Search::SearchStore)
#ifndef AKONADI_SEARCH_NO_PLUGINS
    Q_PLUGIN_METADATA(IID "org.kde.Akonadi.Search.SearchStore" FILE "contactsearchstore.json")
#endif
public:
    explicit ContactSearchStore(QObject *parent = nullptr);

    QStringList types() override;
};
}
}
#endif // AKONADI_SEARCH_PIM_CONTACT_SEARCHSTORE_H
