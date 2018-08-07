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

#include "indexer.h"
#include "registrar_p.h"

#include "email/emailindexer.h"
#include "emailcontacts/emailcontactsindexer.h"
#include "contact/contactindexer.h"
#include "contact/contactgroupindexer.h"
#include "incidence/incidenceindexer.h"
#include "note/noteindexer.h"
#include "collection/collectionindexer.h"

#include <QGlobalStatic>
#include <QHash>

#include <functional>

using namespace Akonadi::Search;

namespace {

Q_GLOBAL_STATIC(Registrar<Indexer>, sIndexers)

} 

Indexer::Indexer()
{
}

Indexer::~Indexer()
{
}

Indexer* Indexer::create(const QString &mimeType)
{
    if (!sIndexers.exists()) {
        sIndexers->registerForType<EmailIndexer>();
        sIndexers->registerForType<EmailContactsIndexer>();
        sIndexers->registerForType<ContactIndexer>();
        sIndexers->registerForType<ContactGroupIndexer>();
        sIndexers->registerForType<IncidenceIndexer>();
        sIndexers->registerForType<NoteIndexer>();
        sIndexers->registerForType<CollectionIndexer>();
    }

    return sIndexers->instantiate(mimeType);
}

QByteArray Indexer::index(const Item &item, const Collection &parent)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    if (doIndex(item, parent, stream)) {
        return buffer;
    }
    return {};
}

QByteArray Indexer::index(const Collection &collection, const Collection &parent)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    if (doIndex(collection, parent, stream)) {
        return buffer;
    }
    return {};
}

bool Indexer::doIndex(const Item &, const Collection &, QDataStream &)
{
    // Assert even in relase mode
    qt_assert_x("Indexer::doIndex(Akonadi::Item)", "Default implementation called!",
                __FILE__, __LINE__);
    return false;
}

bool Indexer::doIndex(const Collection &, const Collection &, QDataStream &)
{
    // Assert event in release mode
    qt_assert_x("Indexer::doIndex(Akonadi::Collection)", "Default implementation called!",
                __FILE__, __LINE__);
    return false;
}
