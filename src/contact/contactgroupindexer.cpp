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
#include "akonadisearch_debug.h"
#include "utils.h"

#include <AkonadiCore/Item>
#include <AkonadiCore/SearchQuery>

#include <KContacts/ContactGroup>

#include <QDataStream>

using namespace Akonadi::Search;

QStringList ContactGroupIndexer::mimeTypes()
{
    return { KContacts::ContactGroup::mimeType() };
}

bool ContactGroupIndexer::doIndex(const Item &item, const Collection &parent, QDataStream &stream)
{
    KContacts::ContactGroup group;
    try {
        group = item.payload<KContacts::ContactGroup>();
    } catch (const Akonadi::PayloadException &e) {
        qCWarning(AKONADISEARCH_LOG) << "Item" << item.id() << "does not contain the expected payload:" << e.what();
        return false;
    }

    XapianDocument doc;

    const QString name = group.name();
    doc.indexText(name);
    doc.indexText(name, ContactQueryPropertyMapper::instance().prefix(Akonadi::ContactSearchTerm::Name));

    // Parent collection
    const auto _parent = parent.isValid() ? parent : item.parentCollection();
    if (!_parent.isValid()) {
        Q_ASSERT_X(_parent.isValid(), "Akonadi::Search::ContactIndexer::index",
                   "Item does not have a valid parent collection");
        return false;
    }
    doc.addCollectionTerm(_parent.id());

    stream << item.id() << doc.xapianDocument();
    return true;
}

