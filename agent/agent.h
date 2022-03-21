/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include <Akonadi/AgentBase>
#include <Akonadi/Collection>

#include "index.h"
#include "scheduler.h"
#include <QList>

class AkonadiIndexingAgent : public Akonadi::AgentBase, public Akonadi::AgentBase::ObserverV3
{
    Q_OBJECT
public:
    using Akonadi::AgentBase::ObserverV3::collectionChanged; // So we don't trigger -Woverloaded-virtual
    explicit AkonadiIndexingAgent(const QString &id);
    ~AkonadiIndexingAgent() override;

    void reindexAll();
    void reindexCollection(const qlonglong id);
    void reindexCollections(const QList<qlonglong> &ids);
    Q_REQUIRED_RESULT qlonglong indexedItems(const qlonglong id);
    Q_REQUIRED_RESULT int numberOfCollectionQueued();

    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers) override;
    void itemsFlagsChanged(const Akonadi::Item::List &items, const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removedFlags) override;
    void itemsRemoved(const Akonadi::Item::List &items) override;
    void itemsMoved(const Akonadi::Item::List &items, const Akonadi::Collection &sourceCollection, const Akonadi::Collection &destinationCollection) override;

    void collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent) override;
    void collectionChanged(const Akonadi::Collection &collection, const QSet<QByteArray> &changedAttributes) override;
    void collectionRemoved(const Akonadi::Collection &collection) override;
    void collectionMoved(const Akonadi::Collection &collection,
                         const Akonadi::Collection &collectionSource,
                         const Akonadi::Collection &collectionDestination) override;

    // Remove the entire db
    void cleanup() override;

private Q_SLOTS:
    void onAbortRequested();
    void onOnlineChanged(bool online);

Q_SIGNALS:
    void collectionIndexingFinished(const qlonglong id);

private:
    Q_REQUIRED_RESULT bool shouldIndex(const Akonadi::Item &item) const;
    Q_REQUIRED_RESULT bool shouldIndex(const Akonadi::Collection &collection) const;

    Index m_index;
    Scheduler m_scheduler;
};
