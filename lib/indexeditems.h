/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2016-2020 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */
#ifndef INDEXEDITEMS_H
#define INDEXEDITEMS_H

#include <QObject>
#include "search_pim_export.h"
#include <AkonadiCore/Item>

namespace Akonadi {
namespace Search {
namespace PIM {
class IndexedItemsPrivate;

/** Indexed items. */
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
