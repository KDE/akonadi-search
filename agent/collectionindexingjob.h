/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */
#pragma once

#include "index.h"
#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <KJob>
#include <QElapsedTimer>

/**
 * Indexing Job that ensure a collections is fully indexed.
 * The following steps are required to bring the index up-to date:
 * 1. Index pending items
 * 2. Check if indexed item == local items (optimization)
 * 3. Make a full diff if necessary
 */
class CollectionIndexingJob : public KJob
{
    Q_OBJECT
public:
    explicit CollectionIndexingJob(Index &index, const Akonadi::Collection &col, const QList<Akonadi::Item::Id> &pending, QObject *parent = nullptr);
    void setFullSync(bool);
    void start() override;

Q_SIGNALS:
    void status(int, const QString &);
    void percent(int);

private Q_SLOTS:
    void slotOnCollectionFetched(KJob *);
    void slotPendingItemsReceived(const Akonadi::Item::List &items);
    void slotPendingIndexed(KJob *);
    void slotUnindexedItemsReceived(const Akonadi::Item::List &items);
    void slotFoundUnindexed(KJob *);

private:
    void findUnindexed();
    void indexItems(const QList<Akonadi::Item::Id> &itemIds);

    Akonadi::Collection m_collection;
    QList<Akonadi::Item::Id> m_pending;
    QSet<Akonadi::Item::Id> m_indexedItems;
    QList<Akonadi::Item::Id> m_needsIndexing;
    Index &m_index;
    QElapsedTimer m_time;
    bool m_reindexingLock = false;
    bool m_fullSync = true;
    int m_progressCounter = 0;
    int m_progressTotal = 0;
};
