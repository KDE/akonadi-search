/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2016-2022 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */
#pragma once

#include "search_pim_export.h"
#include <Akonadi/Item>
#include <QObject>

#include <memory>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
class IndexedItemsPrivate;

/** Indexed items. */
class AKONADI_SEARCH_PIM_EXPORT IndexedItems : public QObject
{
    Q_OBJECT
public:
    explicit IndexedItems(QObject *parent = nullptr);
    ~IndexedItems() override;

    void setOverrideDbPrefixPath(const QString &path);

    qlonglong indexedItems(const qlonglong id);

    void findIndexedInDatabase(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId, const QString &dbPath);
    void findIndexed(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId);

    Q_REQUIRED_RESULT QString emailIndexingPath() const;
    Q_REQUIRED_RESULT QString collectionIndexingPath() const;
    Q_REQUIRED_RESULT QString calendarIndexingPath() const;
    Q_REQUIRED_RESULT QString akonotesIndexingPath() const;
    Q_REQUIRED_RESULT QString emailContactsIndexingPath() const;
    Q_REQUIRED_RESULT QString contactIndexingPath() const;

private:
    std::unique_ptr<IndexedItemsPrivate> const d;
};
}
}
}
