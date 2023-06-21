/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */
#include "collectionindexingjob.h"
#include "abstractindexer.h"
#include <Akonadi/AgentBase>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/CollectionStatistics>
#include <Akonadi/IndexPolicyAttribute>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ServerManager>
#include <KLocalizedString>
#include <akonadi_indexer_agent_debug.h>

CollectionIndexingJob::CollectionIndexingJob(Index &index, const Akonadi::Collection &col, const QList<Akonadi::Item::Id> &pending, QObject *parent)
    : KJob(parent)
    , m_collection(col)
    , m_pending(pending)
    , m_index(index)
{
}

void CollectionIndexingJob::setFullSync(bool enable)
{
    m_fullSync = enable;
}

void CollectionIndexingJob::start()
{
    qCDebug(AKONADI_INDEXER_AGENT_LOG);
    m_time.start();

    // Fetch collection for statistics
    auto job = new Akonadi::CollectionFetchJob(m_collection, Akonadi::CollectionFetchJob::Base);
    job->fetchScope().setIncludeStatistics(true);
    job->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
    job->fetchScope().fetchAttribute<Akonadi::IndexPolicyAttribute>();
    connect(job, &KJob::finished, this, &CollectionIndexingJob::slotOnCollectionFetched);
    job->start();
}

void CollectionIndexingJob::slotOnCollectionFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Failed to fetch items: " << job->errorString();
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }
    m_collection = static_cast<Akonadi::CollectionFetchJob *>(job)->collections().at(0);
    if (m_collection.isVirtual()
        || (m_collection.hasAttribute<Akonadi::IndexPolicyAttribute>() && !m_collection.attribute<Akonadi::IndexPolicyAttribute>()->indexingEnabled())) {
        emitResult();
        return;
    }

    Q_EMIT status(Akonadi::AgentBase::Running, i18n("Indexing collection: %1", m_collection.displayName()));
    Q_EMIT percent(0);

    if (!m_index.haveIndexerForMimeTypes(m_collection.contentMimeTypes())) {
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << "No indexer for collection, skipping";
        emitResult();
        return;
    }

    if (m_pending.isEmpty()) {
        if (!m_fullSync) {
            qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Indexing complete. Total time: " << m_time.elapsed();
            emitResult();
            return;
        }
        findUnindexed();
        return;
    }
    indexItems(m_pending);
}

void CollectionIndexingJob::indexItems(const QList<Akonadi::Item::Id> &itemIds)
{
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "collectionIndexingJob::indexItems(const QList<Akonadi::Item::Id> &itemIds) count " << itemIds.count();
    Akonadi::Item::List items;
    items.reserve(itemIds.size());
    for (const Akonadi::Item::Id id : itemIds) {
        items << Akonadi::Item(id);
    }

    auto fetchJob = new Akonadi::ItemFetchJob(items);
    fetchJob->fetchScope().fetchFullPayload(true);
    fetchJob->fetchScope().setCacheOnly(true);
    fetchJob->fetchScope().setIgnoreRetrievalErrors(true);
    fetchJob->fetchScope().setFetchRemoteIdentification(false);
    fetchJob->fetchScope().setFetchModificationTime(true);
    fetchJob->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    fetchJob->setDeliveryOption(Akonadi::ItemFetchJob::EmitItemsIndividually);
    fetchJob->setProperty("count", items.size());
    fetchJob->setProperty("start", m_time.elapsed());
    m_progressTotal = items.size();
    m_progressCounter = 0;

    connect(fetchJob, &Akonadi::ItemFetchJob::itemsReceived, this, &CollectionIndexingJob::slotPendingItemsReceived);
    connect(fetchJob, &KJob::result, this, &CollectionIndexingJob::slotPendingIndexed);
    fetchJob->start();
}

void CollectionIndexingJob::slotPendingItemsReceived(const Akonadi::Item::List &items)
{
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << " CollectionIndexingJob::slotPendingItemsReceived " << items.count();
    for (const Akonadi::Item &item : items) {
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << " void CollectionIndexingJob::slotPendingItemsReceived(const Akonadi::Item::List &items)" << item.id();
        m_index.index(item);
    }
    m_progressCounter++;
    Q_EMIT percent(100.0 * m_progressCounter / m_progressTotal);
}

void CollectionIndexingJob::slotPendingIndexed(KJob *job)
{
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << " CollectionIndexingJob::slotPendingIndexed ";
    if (job->error()) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Failed to fetch items: " << job->errorString();
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Indexed " << job->property("count").toInt()
                                       << " items in (ms): " << m_time.elapsed() - job->property("start").toInt();

    if (!m_fullSync) {
        m_index.scheduleCommit();
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Indexing complete. Total time: " << m_time.elapsed();
        emitResult();
        return;
    }

    // We need to commit, otherwise the count is not accurate
    m_index.commit();

    const int start = m_time.elapsed();
    const qlonglong indexedItemsCount = m_index.indexedItems(m_collection.id());
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Indexed items count took (ms): " << m_time.elapsed() - start;
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "In index: " << indexedItemsCount;
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Number of Items in collection: " << m_collection.statistics().count() << " In collection " << m_collection.id();

    if (m_collection.statistics().count() == indexedItemsCount) {
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Index up to date";
        emitResult();
        return;
    } else {
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Need to find unindexed items";
    }

    findUnindexed();
}

void CollectionIndexingJob::findUnindexed()
{
    m_indexedItems.clear();
    m_needsIndexing.clear();
    const int start = m_time.elapsed();
    m_index.findIndexed(m_indexedItems, m_collection.id());
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Found " << m_indexedItems.size() << " indexed items. Took (ms): " << m_time.elapsed() - start
                                       << " collection id :" << m_collection.id();

    auto job = new Akonadi::ItemFetchJob(m_collection, this);
    job->fetchScope().fetchFullPayload(false);
    job->fetchScope().setCacheOnly(true);
    job->fetchScope().setIgnoreRetrievalErrors(true);
    job->fetchScope().setFetchRemoteIdentification(false);
    job->fetchScope().setFetchModificationTime(false);
    job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::None);
    job->setDeliveryOption(Akonadi::ItemFetchJob::EmitItemsIndividually);

    connect(job, &Akonadi::ItemFetchJob::itemsReceived, this, &CollectionIndexingJob::slotUnindexedItemsReceived);
    connect(job, &KJob::result, this, &CollectionIndexingJob::slotFoundUnindexed);
    job->start();
}

void CollectionIndexingJob::slotUnindexedItemsReceived(const Akonadi::Item::List &items)
{
    // qCDebug(AKONADI_INDEXER_AGENT_LOG) << "CollectionIndexingJob::slotUnindexedItemsReceived found number items :"<<items.count();
    for (const Akonadi::Item &item : items) {
        if (!m_indexedItems.remove(item.id())) {
            m_needsIndexing << item.id();
        }
    }
}

void CollectionIndexingJob::slotFoundUnindexed(KJob *job)
{
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "CollectionIndexingJob::slotFoundUnindexed :m_needsIndexing.isEmpty() : " << m_needsIndexing.isEmpty()
                                       << " count :" << m_needsIndexing.count() << " m_reindexingLock :" << m_reindexingLock << "m_collection.id() "
                                       << m_collection.id();
    if (job->error()) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Failed to fetch items: " << job->errorString();
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }

    if (!m_indexedItems.isEmpty()) {
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Removing no longer existing items: " << m_indexedItems.size();
        m_index.remove(m_indexedItems, m_collection.contentMimeTypes());
    }
    if (!m_needsIndexing.isEmpty() && !m_reindexingLock) {
        m_reindexingLock = true; // Avoid an endless loop
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Found unindexed: " << m_needsIndexing.size();
        indexItems(m_needsIndexing);
        return;
    }

    m_index.commit();
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Indexing complete. Total time: " << m_time.elapsed();
    emitResult();
}

#include "moc_collectionindexingjob.cpp"
