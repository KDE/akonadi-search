/*
 * Copyright (C) 2018  Daniel Vrátil <dvratil@kde.org>
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

#ifndef AKONADISEARCH_OBJECTCACHE_H_
#define AKONADISEARCH_OBJECTCACHE_H_

#include <QHash>
#include <QString>

namespace Akonadi {
namespace Search {

class Indexer;
class Store;

template<typename T>
class ObjectCache
{
public:
    explicit ObjectCache()
    {}

    ~ObjectCache()
    {
        qDeleteAll(mCache);
    }

    T *get(const QString &mimeType) const
    {
        auto it = mCache.constFind(mimeType);
        if (it == mCache.cend()) {
            auto obj = T::create(mimeType);
            if (obj) {
                return *mCache.insert(mimeType, obj);
            } else {
                return nullptr;
            }
        }
        return *it;
    }

private:
    mutable QHash<QString, T*> mCache;
};

using IndexerCache = ObjectCache<Indexer>;
using StoreCache = ObjectCache<Store>;

}
}

#endif // AKONADISEARCH_OBJECTCACHE_H_
