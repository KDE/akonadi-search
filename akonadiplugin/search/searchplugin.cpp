/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2014  Christian Mollekopf <mollekopf@kolabsys.com>
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

#include "searchplugin.h"

#include <AkonadiCore/SearchQuery>

#include "akonadiplugin_search_debug.h"
#include "store.h"
#include "querymapper.h"
#include "resultiterator.h"

#include <QStringList>
#include <QSet>
#include <QVector>

using namespace Akonadi::Search;

SearchPlugin::QueryMapperStorePair *SearchPlugin::QueryMapperStorePair::create(const QString &mimeType)
{
    QScopedPointer<QueryMapperStorePair> pair(new QueryMapperStorePair);
    pair->queryMapper = QueryMapper::create(mimeType);
    if (!pair->queryMapper) {
        qCWarning(AKONADIPLUGIN_SEARCH_LOG) << "No QueryMapper for type" << mimeType;
        return {};
    }
    pair->store = Store::create(mimeType);
    if (!pair->store) {
        qCWarning(AKONADIPLUGIN_SEARCH_LOG) << "No Store for type" << mimeType;
        return {};
    }
    return pair.take();
}

SearchPlugin::QueryMapperStorePair::~QueryMapperStorePair()
{
    delete queryMapper;
    delete store;
}


SearchPlugin::SearchPlugin()
    : QObject()
{
}

SearchPlugin::~SearchPlugin()
{
}

QSet<qint64> SearchPlugin::search(const QString &akonadiQuery, const QVector<qint64> &collections,
                                  const QStringList &mimeTypes)
{
    if (akonadiQuery.isEmpty() && collections.isEmpty() && mimeTypes.isEmpty()) {
        qCWarning(AKONADIPLUGIN_SEARCH_LOG) << "empty query";
        return {};
    }

    Akonadi::SearchQuery searchQuery;
    if (!akonadiQuery.isEmpty()) {
        searchQuery = Akonadi::SearchQuery::fromJSON(akonadiQuery.toLatin1());
        if (searchQuery.isNull() && collections.isEmpty() && mimeTypes.isEmpty()) {
            return {};
        }
    }

    const Akonadi::SearchTerm term = searchQuery.term();

    //Filter by collection if not empty
    if (!collections.isEmpty()) {
        Akonadi::SearchTerm parentTerm(Akonadi::SearchTerm::RelAnd);
        Akonadi::SearchTerm collectionTerm(Akonadi::SearchTerm::RelOr);
        for (const qint64 col : collections) {
            collectionTerm.addSubTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, col,
                                                          Akonadi::SearchTerm::CondEqual));
        }
        if (term.isNull()) {
            searchQuery.addTerm(collectionTerm);
        } else {
            parentTerm.addSubTerm(collectionTerm);
            parentTerm.addSubTerm(term);
            searchQuery.setTerm(parentTerm);
        }
    } else {
        if (term.isNull()) {
            qCWarning(AKONADIPLUGIN_SEARCH_LOG) << "no terms added";
            return QSet<qint64>();
        }

        searchQuery.setTerm(term);
    }

    QSet<qint64> resultSet;
    for (const auto &mimeType : mimeTypes) {
        if (const auto storePair = mStoreCache.get(mimeType)) {
            const auto query = storePair->queryMapper->map(searchQuery);
            auto iter = storePair->store->search(query, searchQuery.limit());
            while (iter.next()) {
                resultSet << iter.id();
            }
        }
    }
    qCDebug(AKONADIPLUGIN_SEARCH_LOG) << "Got" << resultSet.count() << "results";
    return resultSet;
}
