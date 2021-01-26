/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#ifndef AKONADI_SEARCH_PIM_CONTACT_SEARCHSTORE_H
#define AKONADI_SEARCH_PIM_CONTACT_SEARCHSTORE_H

#include "../pimsearchstore.h"

namespace Akonadi
{
namespace Search
{
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
