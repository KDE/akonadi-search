/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2016-2018 Laurent Montel <montel@kde.org>
 * Copyright (C) 2018 Daniel Vrátil <dvratil@kde.org>
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

#include <xapian.h>

#include "indexeditems.h"
#include "akonadisearch_debug.h"
#include "contact/contactstore.h"
#include "email/emailstore.h"
#include "incidence/incidencestore.h"
#include "note/notestore.h"
#include "querymapper.h"
#include "resultiterator.h"
#include "querypropertymapper_p.h"

#include <AkonadiCore/SearchQuery>

#include <QtConcurrent>
#include <QHash>
#include <QDir>

#include <functional>
#include <memory>

using namespace Akonadi::Search;

namespace {

class HelperQueryPropertyMapper : public QueryPropertyMapper {
public:
    HelperQueryPropertyMapper() = default;
};

class HelperQueryMapper : public QueryMapper {
public:
    HelperQueryMapper() = default;

    const QueryPropertyMapper &propertyMapper() override {
        static HelperQueryPropertyMapper mapper;
        return mapper;
    }
};

}


class Akonadi::Search::IndexedItems::Private
{
public:
    Private() {}

    void findIndexedInStore(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId,
                            const std::unique_ptr<Store> &store) {
        Akonadi::SearchQuery query;
        query.addTerm({Akonadi::SearchTerm::Collection, collectionId});
        auto iter = store->search(HelperQueryMapper().map(query));
        while (iter.next()) {
            indexed.insert(iter.id());
        }
    }
};

IndexedItems::IndexedItems()
    : d(new Private())
{
}

IndexedItems::~IndexedItems()
{
}

qint64 IndexedItems::indexedItems(Akonadi::Collection::Id id)
{
    std::vector<Store>stores{ EmailStore(), ContactStore(), IncidenceStore(), NoteStore() };
    // Qt seems to struggle with deducing the return type of the Map lambda, hence the
    // std::function wrapper.
    return QtConcurrent::blockingMappedReduced<qint64>(stores.begin(), stores.end(),
        std::function<qint64(Store&)>([id](Store &store) { return store.indexedItems(id); }),
        [](qint64 &result, qint64 val) -> qint64 { return result += val; });
}

QSet<Akonadi::Item::Id> IndexedItems::findIndexedForType(Akonadi::Collection::Id collectionId, const QString &mimeType)
{
    QSet<Akonadi::Item::Id> indexed;
    d->findIndexedInStore(indexed, collectionId, std::unique_ptr<Store>(Store::create(mimeType)));
    return indexed;
}

QSet<Akonadi::Item::Id> IndexedItems::findIndexed(Akonadi::Collection::Id collectionId)
{
    // TODO: Paralellize
    QSet<Akonadi::Item::Id> indexed;
    d->findIndexedInStore(indexed, collectionId, std::make_unique<EmailStore>());
    d->findIndexedInStore(indexed, collectionId, std::make_unique<ContactStore>());
    d->findIndexedInStore(indexed, collectionId, std::make_unique<NoteStore>());
    d->findIndexedInStore(indexed, collectionId, std::make_unique<IncidenceStore>());
    return indexed;
}
