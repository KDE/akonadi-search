/*
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "contactgroupindexer.h"
#include "contactquerypropertymapper.h"
#include "xapiandocument.h"

#include <AkonadiCore/Item>
#include <AkonadiCore/SearchQuery>

#include <KContacts/ContactGroup>

using namespace Akonadi::Search;

QStringList ContactGroupIndexer::mimeTypes()
{
    return { KContacts::ContactGroup::mimeType() };
}

Xapian::Document ContactGroupIndexer::index(const Akonadi::Item &item)
{
    KContacts::ContactGroup group;
    try {
        group = item.payload<KContacts::ContactGroup>();
    } catch (const Akonadi::PayloadException &) {
        return {};
    }

    XapianDocument doc;

    const QString name = group.name();
    doc.indexText(name);
    doc.indexText(name, ContactQueryPropertyMapper::instance().prefix(Akonadi::ContactSearchTerm::Name));

    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Akonadi::Search::ContactIndexer::index",
               "Item does not have a valid parent collection");

    const Akonadi::Collection::Id colId = item.parentCollection().id();
    doc.addCollectionTerm(colId);

    return doc.xapianDocument();
}

