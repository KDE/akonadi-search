/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "agent.h"

#include "akonotesindexer.h"
#include "calendarindexer.h"
#include "collectionupdatejob.h"
#include "contactindexer.h"
#include "emailindexer.h"
#include "indexeradaptor.h"

#include "priority.h"

#include <Akonadi/AttributeFactory>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/IndexPolicyAttribute>
#include <Akonadi/ItemFetchScope>

#include <Akonadi/AgentManager>
#include <Akonadi/ServerManager>

#include "akonadi_indexer_agent_debug.h"
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#define INDEXING_AGENT_VERSION 5

AkonadiIndexingAgent::AkonadiIndexingAgent(const QString &id)
    : AgentBase(id)
    , m_scheduler(m_index, config(), QSharedPointer<JobFactory>(new JobFactory))
{
    lowerIOPriority();
    lowerSchedulingPriority();
    lowerPriority();

    Akonadi::AttributeFactory::registerAttribute<Akonadi::IndexPolicyAttribute>();

    KConfigGroup cfg = config()->group("General");
    const int agentIndexingVersion = cfg.readEntry("agentIndexingVersion", 0);
    bool respectDiacriticAndAccents = cfg.readEntry("respectDiacriticAndAccents", true);
    if (agentIndexingVersion < INDEXING_AGENT_VERSION) {
        m_index.removeDatabase();
        // Don't respect Diacritic and Accents in new Database so search will be more easy.
        respectDiacriticAndAccents = false;
        QTimer::singleShot(0, &m_scheduler, &Scheduler::scheduleCompleteSync);
        cfg.writeEntry("agentIndexingVersion", INDEXING_AGENT_VERSION);
        cfg.writeEntry("respectDiacriticAndAccents", respectDiacriticAndAccents);
        cfg.sync();
    }
    m_index.setRespectDiacriticAndAccents(respectDiacriticAndAccents);
    if (!m_index.createIndexers()) {
        Q_EMIT status(Broken, i18nc("@info:status", "No indexers available"));
        setOnline(false);
    } else {
        setOnline(true);
    }
    connect(this, &Akonadi::AgentBase::abortRequested, this, &AkonadiIndexingAgent::onAbortRequested);
    connect(this, &Akonadi::AgentBase::onlineChanged, this, &AkonadiIndexingAgent::onOnlineChanged);

    connect(&m_scheduler, SIGNAL(status(int, QString)), this, SIGNAL(status(int, QString)));
    connect(&m_scheduler, &Scheduler::percent, this, &Akonadi::AgentBase::percent);
    connect(&m_scheduler, &Scheduler::collectionIndexingFinished, this, &AkonadiIndexingAgent::collectionIndexingFinished);

    changeRecorder()->setAllMonitored(true);
    changeRecorder()->itemFetchScope().setCacheOnly(true);
    changeRecorder()->itemFetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    changeRecorder()->itemFetchScope().setFetchRemoteIdentification(false);
    changeRecorder()->itemFetchScope().setFetchModificationTime(false);
    changeRecorder()->itemFetchScope().fetchFullPayload(true);
    changeRecorder()->collectionFetchScope().fetchAttribute<Akonadi::IndexPolicyAttribute>();
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
    for (const Akonadi::AgentInstance &inst : allAgents) {
        if (inst.identifier() == QLatin1String("akonadi_nepomuk_feeder")) {
            qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Removing old nepomuk feeder" << inst.identifier();
            agentManager->removeInstance(inst);
        } else if (inst.identifier() == QLatin1String("akonadi_baloo_indexer")) {
            qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Removing old Baloo indexer" << inst.identifier();
            agentManager->removeInstance(inst);
        }
    }
}

AkonadiIndexingAgent::~AkonadiIndexingAgent() = default;

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

void AkonadiIndexingAgent::reindexCollections(const QList<qlonglong> &ids)
{
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Reindexing collections " << ids;
    for (qlonglong id : ids) {
        m_scheduler.scheduleCollection(Akonadi::Collection(id), true);
    }
}

qlonglong AkonadiIndexingAgent::indexedItems(const qlonglong id)
{
    return m_index.indexedItems(id);
}

void AkonadiIndexingAgent::itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection)
{
    if (!shouldIndex(collection)) {
        return;
    }

    m_scheduler.addItem(item);
}

void AkonadiIndexingAgent::itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers)
{
    if (!shouldIndex(item)) {
        return;
    }

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

void AkonadiIndexingAgent::itemsFlagsChanged(const Akonadi::Item::List &items, const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removedFlags)
{
    // We optimize and skip the "shouldIndex" call for each item here, since it's
    // cheaper to just let Xapian throw an exception for items that were not
    // indexed.
    // In most cases the entire batch comes from the same collection, so we will
    // only suffer penalty in case of larger batches from non-indexed collections,
    // but we assume that that's a much less common case than collections with
    // indexing enabled.

    // Akonadi always sends batch of items of the same type
    m_index.updateFlags(items, addedFlags, removedFlags);
    m_index.scheduleCommit();
}

void AkonadiIndexingAgent::itemsRemoved(const Akonadi::Item::List &items)
{
    // We optimize and skip the "shouldIndex" call for each item here, since it's
    // cheaper to just let Xapian throw an exception for items that were not
    // indexed instead of filtering the list here.

    m_index.remove(items);
    m_index.scheduleCommit();
}

void AkonadiIndexingAgent::itemsMoved(const Akonadi::Item::List &items,
                                      const Akonadi::Collection &sourceCollection,
                                      const Akonadi::Collection &destinationCollection)
{
    const bool indexSource = shouldIndex(sourceCollection);
    const bool indexDest = shouldIndex(destinationCollection);

    if (indexSource && indexDest) {
        m_index.move(items, sourceCollection, destinationCollection);
        m_index.scheduleCommit();
    } else if (!indexSource && indexDest) {
        for (const auto &item : items) {
            m_scheduler.addItem(item);
        }
        m_index.scheduleCommit();
    } else if (indexSource && !indexDest) {
        m_index.remove(items);
        m_index.scheduleCommit();
    } else {
        // nothing to do
    }
}

void AkonadiIndexingAgent::collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent)
{
    Q_UNUSED(parent)

    if (!shouldIndex(collection)) {
        return;
    }

    m_index.index(collection);
    m_index.scheduleCommit();
}

void AkonadiIndexingAgent::collectionChanged(const Akonadi::Collection &collection, const QSet<QByteArray> &changedAttributes)
{
    if (changedAttributes.contains(Akonadi::IndexPolicyAttribute().type())) {
        const auto attr = collection.attribute<Akonadi::IndexPolicyAttribute>();
        if (attr && !attr->indexingEnabled()) {
            // The indexing attribute has changed and is now disabled: remove
            // collection and all indexed items
            m_index.remove(collection);
        } else {
            // The indexing attribute has changed and is now missing or enabled,
            // schedule full collection sync.
            m_scheduler.scheduleCollection(collection, true);
        }
    }

    QSet<QByteArray> changes = changedAttributes;
    changes.remove("collectionquota");
    changes.remove("timestamp");
    changes.remove("imapquota");

    if (changes.isEmpty()) {
        return;
    }

    if (changes.contains("ENTITYDISPLAY")) {
        // If the name changed we have to reindex all subcollections
        auto job = new CollectionUpdateJob(m_index, collection, this);
        job->start();
    } else {
        m_index.index(collection);
        m_index.scheduleCommit();
    }
}

void AkonadiIndexingAgent::collectionRemoved(const Akonadi::Collection &collection)
{
    // We intentionally don't call "shouldIndex" here to make absolutely sure
    // that all items are wiped from the index

    m_index.remove(collection);
    m_index.scheduleCommit();
}

void AkonadiIndexingAgent::collectionMoved(const Akonadi::Collection &collection,
                                           const Akonadi::Collection &collectionSource,
                                           const Akonadi::Collection &collectionDestination)
{
    Q_UNUSED(collectionSource)
    Q_UNUSED(collectionDestination)

    if (!shouldIndex(collection)) {
        return;
    }

    m_index.remove(collection);
    auto job = new CollectionUpdateJob(m_index, collection, this);
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
    KConfigGroup group = config()->group("General");
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
        // We only reindex if this is not a regular start
        KConfigGroup cfg = config()->group("General");
        bool aborted = cfg.readEntry("aborted", false);
        if (aborted) {
            cfg.writeEntry("aborted", false);
            cfg.sync();
            m_scheduler.scheduleCompleteSync();
        }
    } else {
        // Abort ongoing indexing when switched to offline
        onAbortRequested();
    }
}

bool AkonadiIndexingAgent::shouldIndex(const Akonadi::Collection &col) const
{
    return !col.isVirtual() && (!col.hasAttribute<Akonadi::IndexPolicyAttribute>() || col.attribute<Akonadi::IndexPolicyAttribute>()->indexingEnabled());
}

bool AkonadiIndexingAgent::shouldIndex(const Akonadi::Item &item) const
{
    return shouldIndex(item.parentCollection());
}

AKONADI_AGENT_MAIN(AkonadiIndexingAgent)
