/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2017  Daniel Vrátil <dvratil@kde.org>
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

#include "indexingplugin.h"
#include "akonadiplugin_indexing_debug.h"

#include "store.h"

using namespace Akonadi::Search;

IndexingPlugin::IndexingPlugin()
{
}

IndexingPlugin::~IndexingPlugin()
{
    for (auto it = mStoreCache.begin(), end = mStoreCache.end(); it != end; ++it) {
        auto &store = (*it);
        store->commit();
        delete store;
    }
}

bool IndexingPlugin::doIndex(const QString &mimeType, const IndexingFunc &indexFunc)
{
    auto it = mStoreCache.find(mimeType);
    if (it == mStoreCache.end()) {
        const auto store = Store::create(mimeType);
        if (!store) {
            qCWarning(AKONADIPLUGIN_INDEXING_LOG) << "No Store for type" << mimeType;
            return false;
        }
        store->setAutoCommit(100, 2000);
        store->setOpenMode(Store::WriteOnly);
        it = mStoreCache.insert(mimeType, store);
    }

    return indexFunc((*it));
}

bool IndexingPlugin::index(const QString &mimeType, qint64 id, const QByteArray& rawData)
{
    return doIndex(mimeType, [id, &rawData](Store *store) {
        return store->index(id, rawData);
    });
}

bool IndexingPlugin::copy(const QString &mimeType, qint64 sourceId, qint64 sourceCollection,
                          qint64 destId, qint64 destCollection)
{
    return doIndex(mimeType, [sourceId, sourceCollection, destId, destCollection](Store *store) {
        return store->copy(sourceId, sourceCollection, destId, destCollection);
    });
}

bool IndexingPlugin::move(const QString &mimeType, qint64 id, qint64 sourceCollection, qint64 destinationCollection)
{
    return doIndex(mimeType, [id, sourceCollection, destinationCollection](Store *store) {
        return store->move(id, sourceCollection, destinationCollection);
    });
}

bool IndexingPlugin::removeItem(const QString &mimeType, qint64 id)
{
    return doIndex(mimeType, [id](Store *store) {
        return store->removeItem(id);
    });
}

bool IndexingPlugin::removeCollection(const QString &mimeType, qint64 id)
{
    return doIndex(mimeType, [id](Store *store) {
        return store->removeCollection(id);
    });
}
