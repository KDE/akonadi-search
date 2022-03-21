/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */
#pragma once

#include "index.h"
#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <KJob>

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
    Q_REQUIRED_RESULT bool shouldIndex(const Akonadi::Collection &col) const;

    const Akonadi::Collection mCol;
    Index &mIndex;
};
