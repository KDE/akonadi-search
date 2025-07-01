/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "scheduler.h"
using namespace Qt::Literals::StringLiterals;

#include "akonadi_indexer_agent_debug.h"
#include "collectionindexingjob.h"

#include <Akonadi/AgentBase>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/IndexPolicyAttribute>
#include <Akonadi/ServerManager>

#include <KConfigGroup>
#include <KLocalizedString>

#include <QTimer>
#include <chrono>

using namespace std::chrono_literals;

JobFactory::~JobFactory() = default;

CollectionIndexingJob *
JobFactory::createCollectionIndexingJob(Index &index, const Akonadi::Collection &col, const QList<Akonadi::Item::Id> &pending, bool fullSync, QObject *parent)
{
    auto job = new CollectionIndexingJob(index, col, pending, parent);
    job->setFullSync(fullSync);
    return job;
}

Scheduler::Scheduler(Index &index, const KSharedConfigPtr &config, const QSharedPointer<JobFactory> &jobFactory, QObject *parent)
    : QObject(parent)
    , m_config(config)
    , m_index(index)
    , m_jobFactory(jobFactory)
    , m_busyTimeout(5000)
{
    if (!m_jobFactory) {
        m_jobFactory = QSharedPointer<JobFactory>(new JobFactory);
    }
    m_processTimer.setSingleShot(true);
    m_processTimer.setInterval(100ms);
    connect(&m_processTimer, &QTimer::timeout, this, &Scheduler::processNext);

    KConfigGroup cfg = m_config->group(u"General"_s);
    const auto dirtyCollectionsResult2 = cfg.readEntry("dirtyCollections", QList<Akonadi::Collection::Id>());
    m_dirtyCollections = QSet<Akonadi::Collection::Id>(dirtyCollectionsResult2.begin(), dirtyCollectionsResult2.end());

    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Dirty collections " << m_dirtyCollections;
    for (Akonadi::Collection::Id col : std::as_const(m_dirtyCollections)) {
        scheduleCollection(Akonadi::Collection(col), true);
    }

    bool initialIndexingDone = cfg.readEntry("initialIndexingDone", false);
    // Trigger a full sync initially
    if (!initialIndexingDone) {
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << "initial indexing";
        QMetaObject::invokeMethod(this, &Scheduler::scheduleCompleteSync, Qt::QueuedConnection);
    }
    cfg.writeEntry("initialIndexingDone", true);
    cfg.sync();
}

Scheduler::~Scheduler()
{
    collectDirtyCollections();
}

void Scheduler::setBusyTimeout(int timeout)
{
    m_busyTimeout = timeout;
}

int Scheduler::numberOfCollectionQueued() const
{
    return m_collectionQueue.count();
}

void Scheduler::collectDirtyCollections()
{
    KConfigGroup cfg = m_config->group(u"General"_s);
    // Store collections where we did not manage to index all, we'll need to do a full sync for them the next time
    QHash<Akonadi::Collection::Id, QQueue<Akonadi::Item::Id>>::ConstIterator it = m_queues.constBegin();
    QHash<Akonadi::Collection::Id, QQueue<Akonadi::Item::Id>>::ConstIterator end = m_queues.constEnd();
    for (; it != end; ++it) {
        if (!it.value().isEmpty()) {
            m_dirtyCollections.insert(it.key());
        }
    }
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << m_dirtyCollections;
    cfg.writeEntry("dirtyCollections", m_dirtyCollections.values());
    cfg.sync();
}

void Scheduler::scheduleCollection(const Akonadi::Collection &col, bool fullSync)
{
    if (!m_collectionQueue.contains(col.id())) {
        m_collectionQueue.enqueue(col.id());
    }
    if (fullSync) {
        m_dirtyCollections.insert(col.id());
    }
    processNext();
}

void Scheduler::addItem(const Akonadi::Item &item)
{
    Q_ASSERT(item.parentCollection().isValid());
    m_lastModifiedTimestamps.insert(item.parentCollection().id(), QDateTime::currentMSecsSinceEpoch());
    m_queues[item.parentCollection().id()].append(item.id());
    // Move to the back
    m_collectionQueue.removeOne(item.parentCollection().id());
    m_collectionQueue.enqueue(item.parentCollection().id());
    if (!m_processTimer.isActive()) {
        m_processTimer.start();
    }
}

void Scheduler::scheduleCompleteSync()
{
    qCDebug(AKONADI_INDEXER_AGENT_LOG);
    {
        auto job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive);
        job->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
        job->fetchScope().setListFilter(Akonadi::CollectionFetchScope::Index);
        job->fetchScope().fetchAttribute<Akonadi::IndexPolicyAttribute>();
        connect(job, &KJob::finished, this, &Scheduler::slotRootCollectionsFetched);
        job->start();
    }

    // We want to index all collections, even if we don't index their content
    {
        auto job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive);
        job->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
        job->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
        job->fetchScope().setListFilter(Akonadi::CollectionFetchScope::Index);
        connect(job, &KJob::finished, this, &Scheduler::slotCollectionsToIndexFetched);
        job->start();
    }
}

void Scheduler::slotRootCollectionsFetched(KJob *kjob)
{
    auto cjob = static_cast<Akonadi::CollectionFetchJob *>(kjob);
    const Akonadi::Collection::List lstCols = cjob->collections();
    for (const Akonadi::Collection &c : lstCols) {
        // For skipping search collections
        if (c.isVirtual()) {
            continue;
        }
        if (c == Akonadi::Collection::root()) {
            continue;
        }
        if (c.hasAttribute<Akonadi::IndexPolicyAttribute>() && !c.attribute<Akonadi::IndexPolicyAttribute>()->indexingEnabled()) {
            continue;
        }
        scheduleCollection(c, true);
    }

    // If we did not schedule any collection
    if (m_collectionQueue.isEmpty()) {
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << "No collections scheduled";
        Q_EMIT status(Akonadi::AgentBase::Idle, i18n("Ready"));
    }
}

void Scheduler::slotCollectionsToIndexFetched(KJob *kjob)
{
    auto cjob = static_cast<Akonadi::CollectionFetchJob *>(kjob);
    const Akonadi::Collection::List lstCols = cjob->collections();
    for (const Akonadi::Collection &c : lstCols) {
        // For skipping search collections
        if (c.isVirtual()) {
            continue;
        }
        if (c == Akonadi::Collection::root()) {
            continue;
        }
        if (c.hasAttribute<Akonadi::IndexPolicyAttribute>() && !c.attribute<Akonadi::IndexPolicyAttribute>()->indexingEnabled()) {
            continue;
        }
        m_index.index(c);
    }
}

void Scheduler::abort()
{
    if (m_currentJob) {
        m_currentJob->kill(KJob::Quietly);
    }
    m_currentJob = nullptr;
    collectDirtyCollections();
    m_collectionQueue.clear();
    Q_EMIT status(Akonadi::AgentBase::Idle, i18n("Ready"));
}

void Scheduler::processNext()
{
    m_processTimer.stop();
    if (m_currentJob) {
        return;
    }
    if (m_collectionQueue.isEmpty()) {
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Processing done";
        Q_EMIT status(Akonadi::AgentBase::Idle, i18n("Ready"));
        return;
    }

    // An item was queued within the last 5 seconds, we're probably in the middle of a sync
    const bool collectionIsChanging = (QDateTime::currentMSecsSinceEpoch() - m_lastModifiedTimestamps.value(m_collectionQueue.head())) < m_busyTimeout;
    if (collectionIsChanging) {
        // We're in the middle of something, wait with indexing
        m_processTimer.start();
        return;
    }

    const Akonadi::Collection col(m_collectionQueue.takeFirst());
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Processing collection:" << col.id();
    QQueue<Akonadi::Item::Id> &itemQueue = m_queues[col.id()];
    const bool fullSync = m_dirtyCollections.contains(col.id());
    CollectionIndexingJob *job = m_jobFactory->createCollectionIndexingJob(m_index, col, itemQueue, fullSync, this);
    itemQueue.clear();
    job->setProperty("collection", col.id());
    connect(job, &KJob::result, this, &Scheduler::slotIndexingFinished);
    connect(job, &CollectionIndexingJob::status, this, &Scheduler::status);
    connect(job, SIGNAL(percent(int)), this, SIGNAL(percent(int)));
    m_currentJob = job;
    job->start();
}

void Scheduler::slotIndexingFinished(KJob *job)
{
    if (job->error()) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Indexing failed:" << job->errorString();
    } else {
        const auto collectionId = job->property("collection").value<Akonadi::Collection::Id>();
        m_dirtyCollections.remove(collectionId);
        Q_EMIT status(Akonadi::AgentBase::Idle, i18n("Collection \"%1\" indexed", collectionId));
        Q_EMIT collectionIndexingFinished(collectionId);
    }
    m_currentJob = nullptr;
    m_processTimer.start();
}

#include "moc_scheduler.cpp"
