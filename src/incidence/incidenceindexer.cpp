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

#include "incidenceindexer.h"
#include "incidencequerypropertymapper.h"
#include "akonadisearch_debug.h"
#include "xapiandocument.h"
#include "utils.h"

#include <AkonadiCore/Item>
#include <AkonadiCore/SearchQuery>

using namespace Akonadi::Search;

QStringList IncidenceIndexer::mimeTypes()
{
    return { KCalCore::Event::eventMimeType(),
             KCalCore::Todo::todoMimeType(),
             KCalCore::Journal::journalMimeType() };
}

bool IncidenceIndexer::doIndex(const Item &item, const Collection &parent, QDataStream &stream)
{
    Xapian::Document xapDoc;
    if (!item.hasPayload()) {
        qCWarning(AKONADISEARCH_LOG) << "Item" << item.id() << "does not contain the requested payload: No payload set";
        return false;
    }

    if (item.hasPayload<KCalCore::Event::Ptr>()) {
        xapDoc = indexEvent(item.payload<KCalCore::Event::Ptr>());
    } else if (item.hasPayload<KCalCore::Todo::Ptr>()) {
        xapDoc = indexTodo(item.payload<KCalCore::Todo::Ptr>());
    } else if (item.hasPayload<KCalCore::Journal::Ptr>()) {
        xapDoc = indexJournal(item.payload<KCalCore::Journal::Ptr>());
    } else {
        const auto mtids = item.availablePayloadMetaTypeIds();
        QStringList mts;
        for (const auto mtid : mtids) {
            mts << QString::fromUtf8(QMetaType::typeName(mtid));
        }
        qCWarning(AKONADISEARCH_LOG) << "Item" << item.id() << "does not contain the requested payload: "
                                     << "Requested: KCalCore::Event, Todo or Journal, present:" << mts;
        return false;
    }

    // Parent collection
    const auto _parent = parent.isValid() ? parent : item.parentCollection();
    if (!_parent.isValid()) {
        Q_ASSERT_X(_parent.isValid(), "Akonadi::Search::CalenderIndexer::index",
                   "Item does not have a valid parent collection");
        return false;
    }

    XapianDocument doc(xapDoc);
    doc.addCollectionTerm(_parent.id());

    stream << item.id() << doc.xapianDocument();
    return true;
}

Xapian::Document IncidenceIndexer::indexEvent(const KCalCore::Event::Ptr& event)
{
    const auto &propMapper = IncidenceQueryPropertyMapper::instance();

    XapianDocument doc;

    doc.indexText(event->organizer()->email(), propMapper.prefix(Akonadi::IncidenceSearchTerm::Organizer));
    doc.indexText(event->summary(), propMapper.prefix(Akonadi::IncidenceSearchTerm::Summary));
    doc.indexText(event->location(), propMapper.prefix(Akonadi::IncidenceSearchTerm::Location));
    KCalCore::Attendee::List attendees = event->attendees();
    KCalCore::Attendee::List::ConstIterator it;
    KCalCore::Attendee::List::ConstIterator end(attendees.constEnd());
    for (it = attendees.constBegin(); it != end; ++it) {
        doc.addBoolTerm((*it)->email() + QString::number((*it)->status()),
                        propMapper.prefix(Akonadi::IncidenceSearchTerm::PartStatus));
    }


    return doc.xapianDocument();
}

Xapian::Document IncidenceIndexer::indexTodo(const KCalCore::Todo::Ptr &)
{
    // TODO
    return {};
}

Xapian::Document IncidenceIndexer::indexJournal(const KCalCore::Journal::Ptr &)
{
    // TODO
    return {};
}
