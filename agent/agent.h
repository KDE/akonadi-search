/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2012  Vishesh Handa <me@vhanda.in>
 * Copyright (C) 2014  Christian Mollekopf <mollekopf@kolabsys.com>
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
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AGENT_H
#define AGENT_H

#include <agentbase.h>
#include <collection.h>

#include <QList>
#include "scheduler.h"
#include "index.h"


class AkonadiIndexingAgent : public Akonadi::AgentBase, public Akonadi::AgentBase::ObserverV3
{
    Q_OBJECT
public:
    using Akonadi::AgentBase::ObserverV3::collectionChanged; // So we don't trigger -Woverloaded-virtual
    explicit AkonadiIndexingAgent(const QString &id);
    ~AkonadiIndexingAgent();

    void reindexAll();
    void reindexCollection(const qlonglong id);
    void reindexCollections(const QList<qlonglong> &ids);
    qlonglong indexedItems(const qlonglong id);
    int numberOfCollectionQueued();

    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers) override;
    void itemsFlagsChanged(const Akonadi::Item::List &items,
                           const QSet<QByteArray> &addedFlags,
                           const QSet<QByteArray> &removedFlags) override;
    void itemsRemoved(const Akonadi::Item::List &items) override;
    void itemsMoved(const Akonadi::Item::List &items,
                    const Akonadi::Collection &sourceCollection,
                    const Akonadi::Collection &destinationCollection) override;

    void collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent) override;
    void collectionChanged(const Akonadi::Collection &collection, const QSet<QByteArray> &changedAttributes) override;
    void collectionRemoved(const Akonadi::Collection &collection) override;
    void collectionMoved(const Akonadi::Collection &collection, const Akonadi::Collection &collectionSource,
                         const Akonadi::Collection &collectionDestination) override;

    // Remove the entire db
    void cleanup() override;

private Q_SLOTS:
    void onAbortRequested();
    void onOnlineChanged(bool online);

Q_SIGNALS:
    void collectionIndexingFinished(const qlonglong id);

private:
    bool shouldIndex(const Akonadi::Item &item) const;
    bool shouldIndex(const Akonadi::Collection &collection) const;

    Index m_index;
    Scheduler m_scheduler;
};

#endif // AGENT_H
