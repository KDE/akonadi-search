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

CollectionUpdateJob::CollectionUpdateJob(Index &index, const Akonadi::Collection &col, QObject *parent)
    : KJob(parent),
      mCol(col),
      mIndex(index)
{

}

void CollectionUpdateJob::start()
{
    mIndex.change(mCol);

    //Fetch children to update the path accordingly
    Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(mCol, Akonadi::CollectionFetchJob::Recursive, this);
    fetchJob->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
    fetchJob->fetchScope().ancestorFetchScope().fetchAttribute<Akonadi::EntityDisplayAttribute>();
    fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
    connect(fetchJob, SIGNAL(collectionsReceived(Akonadi::Collection::List)), this, SLOT(onCollectionsReceived(Akonadi::Collection::List)));
    connect(fetchJob, SIGNAL(result(KJob*)), this, SLOT(onCollectionsFetched(KJob*)));
}

void CollectionUpdateJob::onCollectionsReceived(const Akonadi::Collection::List &list)
{
    //Required to update the path
    Q_FOREACH (const Akonadi::Collection &child, list) {
        mIndex.change(child);
    }
}

void CollectionUpdateJob::onCollectionsFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << job->errorString();
    }
    emitResult();
}

