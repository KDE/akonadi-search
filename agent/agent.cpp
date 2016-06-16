/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "agent.h"

#include "contactindexer.h"
#include "emailindexer.h"
#include "akonotesindexer.h"
#include "calendarindexer.h"
#include "indexeradaptor.h"
#include "collectionupdatejob.h"

#include "priority.h"

#include <ChangeRecorder>
#include <AkonadiCore/CollectionFetchScope>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/EntityDisplayAttribute>

#include <AgentManager>
#include <ServerManager>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include "akonadi_indexer_agent_debug.h"

#include <QFile>

#define INDEXING_AGENT_VERSION 4

AkonadiIndexingAgent::AkonadiIndexingAgent(const QString &id)
    : AgentBase(id),
      m_scheduler(m_index, QSharedPointer<JobFactory>(new JobFactory))
{
    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    // TODO: Migrate from baloorc to custom config file
    KConfig config(QStringLiteral("baloorc"));
    KConfigGroup group = config.group("Akonadi");
    const int agentIndexingVersion = group.readEntry("agentIndexingVersion", 0);
    if (agentIndexingVersion < INDEXING_AGENT_VERSION) {
        m_index.removeDatabase();
        QTimer::singleShot(0, &m_scheduler, &Scheduler::scheduleCompleteSync);
        group.writeEntry("agentIndexingVersion", INDEXING_AGENT_VERSION);
        group.sync();
    }

    if (!m_index.createIndexers()) {
        Q_EMIT status(Broken, i18nc("@info:status", "No indexers available"));
        setOnline(false);
    } else {
        setOnline(true);
    }
    connect(this, &Akonadi::AgentBase::abortRequested,
            this, &AkonadiIndexingAgent::onAbortRequested);
    connect(this, &Akonadi::AgentBase::onlineChanged,
            this, &AkonadiIndexingAgent::onOnlineChanged);

    connect(&m_scheduler, SIGNAL(status(int,QString)), this, SIGNAL(status(int,QString)));
    connect(&m_scheduler, &Scheduler::percent, this, &Akonadi::AgentBase::percent);

    changeRecorder()->setAllMonitored(true);
    changeRecorder()->itemFetchScope().setCacheOnly(true);
    changeRecorder()->itemFetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    changeRecorder()->itemFetchScope().setFetchRemoteIdentification(false);
    changeRecorder()->itemFetchScope().setFetchModificationTime(false);
    changeRecorder()->collectionFetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
    changeRecorder()->collectionFetchScope().ancestorFetchScope().fetchAttribute<Akonadi::EntityDisplayAttribute>();
    changeRecorder()->collectionFetchScope().setListFilter(Akonadi::CollectionFetchScope::Index);
    changeRecorder()->setChangeRecordingEnabled(false);
    changeRecorder()->fetchCollection(true);
    changeRecorder()->setExclusive(true);

    new IndexerAdaptor(this);

    // Cleanup agentsrc after migration to 4.13/KF5
    Akonadi::AgentManager *agentManager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List allAgents = agentManager->instances();
    // Cannot use agentManager->instance(oldInstanceName) here, it wouldn't find broken instances.
    Q_FOREACH (const Akonadi::AgentInstance &inst, allAgents) {
        if (inst.identifier() == QLatin1String("akonadi_nepomuk_feeder")) {
            qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Removing old nepomuk feeder" << inst.identifier();
            agentManager->removeInstance( inst );
        } else if (inst.identifier() == QLatin1String("akonadi_baloo_indexer")) {
            qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Removing old Baloo indexer" << inst.identifier();
            agentManager->removeInstance(inst);
        }
    }
}

AkonadiIndexingAgent::~AkonadiIndexingAgent()
{
}

void AkonadiIndexingAgent::reindexAll()
{
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Reindexing everything";
    m_scheduler.abort();
    m_index.removeDatabase();
    m_index.createIndexers();
    QTimer::singleShot(0, &m_scheduler, &Scheduler::scheduleCompleteSync);
}

void AkonadiIndexingAgent::reindexCollection(const qlonglong id)
{
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Reindexing collection " << id;
    m_scheduler.scheduleCollection(Akonadi::Collection(id), true);
}

qlonglong AkonadiIndexingAgent::indexedItems(const qlonglong id)
{
    return m_index.indexedItems(id);
}

void AkonadiIndexingAgent::itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection)
{
    Q_UNUSED(collection);
    m_scheduler.addItem(item);
}

void AkonadiIndexingAgent::itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers)
{
    // We don't index certain parts so we don't care when they change
    QSet<QByteArray> pi = partIdentifiers;
    QMutableSetIterator<QByteArray> it(pi);
    while (it.hasNext()) {
        it.next();
        if (!it.value().startsWith("PLD:")) {
            it.remove();
        }
    }

    if (pi.isEmpty()) {
        return;
    }
    m_scheduler.addItem(item);
}

void AkonadiIndexingAgent::itemsFlagsChanged(const Akonadi::Item::List &items,
                                             const QSet<QByteArray> &addedFlags,
                                             const QSet<QByteArray> &removedFlags)
{
    // Akonadi always sends batch of items of the same type
    m_index.updateFlags(items, addedFlags, removedFlags);
    m_index.scheduleCommit();
}

void AkonadiIndexingAgent::itemsRemoved(const Akonadi::Item::List &items)
{
    m_index.remove(items);
    m_index.scheduleCommit();
}

void AkonadiIndexingAgent::itemsMoved(const Akonadi::Item::List &items,
                                      const Akonadi::Collection &sourceCollection,
                                      const Akonadi::Collection &destinationCollection)
{
    m_index.move(items, sourceCollection, destinationCollection);
    m_index.scheduleCommit();
}

void AkonadiIndexingAgent::collectionAdded(const Akonadi::Collection &collection,
                                           const Akonadi::Collection &parent)
{
    Q_UNUSED(parent);
    m_index.index(collection);
    m_index.scheduleCommit();
}

void AkonadiIndexingAgent::collectionChanged(const Akonadi::Collection &collection,
                                             const QSet<QByteArray> &changedAttributes)
{
    QSet<QByteArray> changes = changedAttributes;
    changes.remove("collectionquota");
    changes.remove("timestamp");
    changes.remove("imapquota");

    if (changes.isEmpty()) {
        return;
    }

    if (changes.contains("ENTITYDISPLAY")) {
        //If the name changed we have to reindex all subcollections
        CollectionUpdateJob *job = new CollectionUpdateJob(m_index, collection, this);
        job->start();
    } else {
        m_index.index(collection);
        m_index.scheduleCommit();
    }
}

void AkonadiIndexingAgent::collectionRemoved(const Akonadi::Collection &collection)
{
    m_index.remove(collection);
    m_index.scheduleCommit();
}

void AkonadiIndexingAgent::collectionMoved(const Akonadi::Collection &collection,
                                           const Akonadi::Collection &collectionSource,
                                           const Akonadi::Collection &collectionDestination)
{
    Q_UNUSED(collectionSource);
    Q_UNUSED(collectionDestination);

    m_index.remove(collection);
    CollectionUpdateJob *job = new CollectionUpdateJob(m_index, collection, this);
    job->start();
}

void AkonadiIndexingAgent::cleanup()
{
    // Remove all the databases
    Akonadi::AgentBase::cleanup();
}

int AkonadiIndexingAgent::numberOfCollectionQueued()
{
    return m_scheduler.numberOfCollectionQueued();
}

void AkonadiIndexingAgent::onAbortRequested()
{
    KConfig config(QStringLiteral("baloorc"));
    KConfigGroup group = config.group("Akonadi");
    group.writeEntry("aborted", true);
    group.sync();
    m_scheduler.abort();
}

void AkonadiIndexingAgent::onOnlineChanged(bool online)
{
    // Ignore everything when offline
    changeRecorder()->setAllMonitored(online);

    // Index items that might have changed while we were offline
    if (online) {
        //We only reindex if this is not a regular start
        KConfig config(QStringLiteral("baloorc"));
        KConfigGroup group = config.group("Akonadi");
        const bool aborted = group.readEntry("aborted", false);
        if (aborted) {
            group.writeEntry("aborted", false);
            group.sync();
            m_scheduler.scheduleCompleteSync();
        }
    } else {
        // Abort ongoing indexing when switched to offline
        onAbortRequested();
    }
}

AKONADI_AGENT_MAIN(AkonadiIndexingAgent)
