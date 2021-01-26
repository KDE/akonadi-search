/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */
#include "collectionupdatejob.h"
#include "akonadi_indexer_agent_debug.h"

#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>
#include <AkonadiCore/EntityDisplayAttribute>
#include <AkonadiCore/IndexPolicyAttribute>

CollectionUpdateJob::CollectionUpdateJob(Index &index, const Akonadi::Collection &col, QObject *parent)
    : KJob(parent)
    , mCol(col)
    , mIndex(index)
{
}

void CollectionUpdateJob::start()
{
    if (shouldIndex(mCol)) {
        mIndex.change(mCol);
    }

    // Fetch children to update the path accordingly
    auto *fetchJob = new Akonadi::CollectionFetchJob(mCol, Akonadi::CollectionFetchJob::Recursive, this);
    fetchJob->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
    fetchJob->fetchScope().ancestorFetchScope().fetchAttribute<Akonadi::EntityDisplayAttribute>();
    fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
    fetchJob->fetchScope().fetchAttribute<Akonadi::IndexPolicyAttribute>();
    connect(fetchJob, &Akonadi::CollectionFetchJob::collectionsReceived, this, &CollectionUpdateJob::onCollectionsReceived);
    connect(fetchJob, &KJob::result, this, &CollectionUpdateJob::onCollectionsFetched);
}

void CollectionUpdateJob::onCollectionsReceived(const Akonadi::Collection::List &list)
{
    // Required to update the path
    for (const Akonadi::Collection &child : list) {
        if (shouldIndex(mCol)) {
            mIndex.change(child);
        }
    }
}

void CollectionUpdateJob::onCollectionsFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << job->errorString();
    }
    emitResult();
}

bool CollectionUpdateJob::shouldIndex(const Akonadi::Collection &col) const
{
    return !col.isVirtual() && (!mCol.hasAttribute<Akonadi::IndexPolicyAttribute>() || mCol.attribute<Akonadi::IndexPolicyAttribute>()->indexingEnabled());
}
