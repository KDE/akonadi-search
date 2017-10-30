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
#include "contact/contactindexer.h"
#include "contact/contactgroupindexer.h"
#include "incidence/incidenceindexer.h"
#include "note/noteindexer.h"

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

QVector<Indexer*> Indexer::create(const QString &mimeType)
{
    if (!sIndexers.exists()) {
        sIndexers->registerForType<EmailIndexer>();
        sIndexers->registerForType<ContactIndexer>();
        sIndexers->registerForType<ContactGroupIndexer>();
        sIndexers->registerForType<IncidenceIndexer>();
        sIndexers->registerForType<NoteIndexer>();
    }

    return sIndexers->instantiate(mimeType);
}

QByteArray Indexer::index(const Akonadi::Item &item)
{
    const auto serialized = doIndex(item).serialise();
    return QByteArray(serialized.c_str(), serialized.size());
}

QByteArray Akonadi::Search::Indexer::index(const Akonadi::Collection& collection)
{
    const auto serialized = doIndex(collection).serialise();
    return QByteArray(serialized.c_str(), serialized.size());
}


Xapian::Document Indexer::doIndex(const Akonadi::Item &)
{
    Q_ASSERT_X(false, "Indexer::doIndex(Akonadi::Item)", "Default implementation called!");
    return {};
}

Xapian::Document Indexer::doIndex(const Akonadi::Collection &)
{
    Q_ASSERT_X(false, "Indexer::doIndex(Akonadi::Collection)", "Default implementation called!");
    return {};
}
