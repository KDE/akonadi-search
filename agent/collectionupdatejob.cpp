/*
 * Copyright 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "collectionupdatejob.h"
#include "akonadi_indexer_agent_debug.h"

#include <AkonadiCore/Collection>
#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>
#include <AkonadiCore/EntityDisplayAttribute>
#include <AkonadiCore/IndexPolicyAttribute>

CollectionUpdateJob::CollectionUpdateJob(Index &index, const Akonadi::Collection &col, QObject *parent)
    : KJob(parent),
      mCol(col),
      mIndex(index)
{

}

void CollectionUpdateJob::start()
{
    if (shouldIndex(mCol)) {
        mIndex.change(mCol);
    }

    //Fetch children to update the path accordingly
    Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(mCol, Akonadi::CollectionFetchJob::Recursive, this);
    fetchJob->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
    fetchJob->fetchScope().ancestorFetchScope().fetchAttribute<Akonadi::EntityDisplayAttribute>();
    fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
    fetchJob->fetchScope().fetchAttribute<Akonadi::IndexPolicyAttribute>();
    connect(fetchJob, &Akonadi::CollectionFetchJob::collectionsReceived, this, &CollectionUpdateJob::onCollectionsReceived);
    connect(fetchJob, &KJob::result, this, &CollectionUpdateJob::onCollectionsFetched);
}

void CollectionUpdateJob::onCollectionsReceived(const Akonadi::Collection::List &list)
{
    //Required to update the path
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
    return !col.isVirtual()
           && (!mCol.hasAttribute<Akonadi::IndexPolicyAttribute>()
               || mCol.attribute<Akonadi::IndexPolicyAttribute>()->indexingEnabled());
}
