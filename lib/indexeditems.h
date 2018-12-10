/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2016-2018 Laurent Montel <montel@kde.org>
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
#ifndef INDEXEDITEMS_H
#define INDEXEDITEMS_H

#include <QObject>
#include "search_pim_export.h"
#include <AkonadiCore/Item>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
class IndexedItemsPrivate;
class AKONADI_SEARCH_PIM_EXPORT IndexedItems : public QObject
{
    Q_OBJECT
public:
    explicit IndexedItems(QObject *parent = nullptr);
    ~IndexedItems();

    void setOverrideDbPrefixPath(const QString &path);

    qlonglong indexedItems(const qlonglong id);

    void findIndexedInDatabase(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId, const QString &dbPath);
    void findIndexed(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId);

    QString emailIndexingPath() const;
    QString collectionIndexingPath() const;
    QString calendarIndexingPath() const;
    QString akonotesIndexingPath() const;
    QString emailContactsIndexingPath() const;
    QString contactIndexingPath() const;

private:
    IndexedItemsPrivate *const d;
};
}
}
}
#endif // INDEXEDITEMS_H
