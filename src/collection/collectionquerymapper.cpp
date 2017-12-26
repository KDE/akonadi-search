/*
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

#include <xapian.h>

#include "querymapper_p.h"
#include "collectionquerymapper.h"
#include "collectionquerypropertymapper.h"

#include <AkonadiCore/Collection>
#include <AkonadiCore/SearchQuery>

using namespace Akonadi::Search;

CollectionQueryMapper::CollectionQueryMapper()
{
}

QStringList CollectionQueryMapper::mimeTypes()
{
    return { Akonadi::Collection::mimeType() };
}

const QueryPropertyMapper &CollectionQueryMapper::propertyMapper()
{
    return CollectionQueryPropertyMapper::instance();
}

Xapian::Query CollectionQueryMapper::recursiveTermMapping(const Akonadi::SearchTerm &term)
{
    const auto field = Akonadi::CollectionSearchTerm::fromKey(term.key());
    switch (field) {
    case Akonadi::CollectionSearchTerm::Name:
    case Akonadi::CollectionSearchTerm::MimeType:
    case Akonadi::CollectionSearchTerm::Namespace:
    case Akonadi::CollectionSearchTerm::Identification:
        return constructQuery(propertyMapper(), field, term);
    default:
        return QueryMapper::recursiveTermMapping(term);
    }

    return {};
}
