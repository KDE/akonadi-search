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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef COLLECTIONUPDATEJOB_H
#define COLLECTIONUPDATEJOB_H

#include <KJob>
#include <AkonadiCore/Item>
#include <AkonadiCore/Collection>
#include "index.h"

/**
 * A Job that indexes a collection and all it's children in order to correctly update the paths (for which we need to have the display attribute available).
 */
class CollectionUpdateJob : public KJob
{
    Q_OBJECT
public:
    explicit CollectionUpdateJob(Index &index, const Akonadi::Collection &col, QObject *parent = nullptr);

    void start() override;

private Q_SLOTS:
    void onCollectionsReceived(const Akonadi::Collection::List &);
    void onCollectionsFetched(KJob *);

private:
    bool shouldIndex(const Akonadi::Collection &col) const;

    Akonadi::Collection mCol;
    Index &mIndex;
};

#endif

