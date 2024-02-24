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
#include <KSharedConfig>
#include <QObject>
#include <QQueue>

class CollectionIndexingJob;

class JobFactory
{
public:
    virtual ~JobFactory();
    virtual CollectionIndexingJob *createCollectionIndexingJob(Index &index,
                                                               const Akonadi::Collection &col,
                                                               const QList<Akonadi::Item::Id> &pending,
                                                               bool fullSync,
                                                               QObject *parent = nullptr);
};

/**
 * The scheduler is responsible for scheduling all scheduled tasks.
 *
 * In normal operation this simply involves indexing items and collections that have been added.
 *
 * The scheduler automatically remembers if we failed to index some items before shutting down, and
 * issues a full sync for the affected collections.
 */
class Scheduler : public QObject
{
    Q_OBJECT
public:
    explicit Scheduler(Index &index,
                       const KSharedConfigPtr &config,
                       const QSharedPointer<JobFactory> &jobFactory = QSharedPointer<JobFactory>(),
                       QObject *parent = nullptr);
    ~Scheduler() override;
    void addItem(const Akonadi::Item &);
    void scheduleCollection(const Akonadi::Collection &, bool fullSync = false);

    void abort();

    /**
     * Sets the timeout used to detect when a collection is no longer busy (in ms). Used for testing.
     * Default is 5000.
     */
    void setBusyTimeout(int);

    [[nodiscard]] int numberOfCollectionQueued() const;

Q_SIGNALS:
    void status(int status, const QString &message = QString());
    void percent(int);
    void collectionIndexingFinished(Akonadi::Collection::Id id);

public Q_SLOTS:
    void scheduleCompleteSync();

private:
    void processNext();
    void slotIndexingFinished(KJob *);
    void slotRootCollectionsFetched(KJob *);
    void slotCollectionsToIndexFetched(KJob *);
    void collectDirtyCollections();

    KSharedConfigPtr m_config;
    QHash<Akonadi::Collection::Id, QQueue<Akonadi::Item::Id>> m_queues;
    QQueue<Akonadi::Collection::Id> m_collectionQueue;
    Index &m_index;
    KJob *m_currentJob = nullptr;
    QTimer m_processTimer;
    QHash<Akonadi::Collection::Id, qint64> m_lastModifiedTimestamps;
    QSet<Akonadi::Collection::Id> m_dirtyCollections;
    QSharedPointer<JobFactory> m_jobFactory;
    int m_busyTimeout;
};
