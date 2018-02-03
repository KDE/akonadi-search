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

#include "collectionindexer.h"
#include "collectionquerypropertymapper.h"
#include "xapiandocument.h"
#include "akonadisearch_debug.h"

#include <AkonadiCore/Collection>
#include <AkonadiCore/SearchQuery>
#include <AkonadiCore/CollectionIdentificationAttribute>

using namespace Akonadi::Search;

QStringList CollectionIndexer::mimeTypes()
{
    return { Collection::mimeType() };
}

Xapian::Document CollectionIndexer::doIndex(const Collection &col, const Collection &parent)
{
    XapianDocument doc;

    const auto &propMapper = CollectionQueryPropertyMapper::instance();

    doc.indexText(col.displayName(), propMapper.prefix(Akonadi::CollectionSearchTerm::Name), 1);
    doc.indexTextWithoutPositions(col.displayName(), {}, 100);
    const auto mimeTypes = col.contentMimeTypes();
    for (const auto &mt : mimeTypes) {
        doc.indexText(mt, propMapper.prefix(Akonadi::CollectionSearchTerm::MimeType), 1);
    }

    if (auto attr = col.attribute<Akonadi::CollectionIdentificationAttribute>()) {
        if (!attr->collectionNamespace().isEmpty()) {
            doc.addBoolTerm(QString::fromUtf8(attr->collectionNamespace()),
                            propMapper.prefix(Akonadi::CollectionSearchTerm::Namespace));
        }
        if (!attr->identifier().isEmpty()) {
            doc.addBoolTerm(QString::fromUtf8(attr->identifier()),
                            propMapper.prefix(Akonadi::CollectionSearchTerm::Identification));
        }
    }

    const auto _parent = parent.isValid() ? parent : col.parentCollection();
    if (!_parent.isValid()) {
        Q_ASSERT_X(_parent.isValid(), "Akonadi::Search::CollectionIndexer::index",
                   "Collection does not have a valid parent collection");
        return {};
    }
    doc.addCollectionTerm(_parent.id());

    return doc.xapianDocument();
}

