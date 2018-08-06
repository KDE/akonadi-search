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
#include "akonadisearch_debug.h"
#include "xapiandocument.h"

#include <AkonadiCore/Item>

using namespace Akonadi::Search;

QStringList IncidenceIndexer::mimeTypes()
{
    return { KCalCore::Event::eventMimeType(),
             KCalCore::Todo::todoMimeType(),
             KCalCore::Journal::journalMimeType() };
}

Xapian::Document IncidenceIndexer::index(const Akonadi::Item &item)
{
    Xapian::Document xapDoc;
    if (item.hasPayload<KCalCore::Event::Ptr>()) {
        xapDoc = indexEvent(item.payload<KCalCore::Event::Ptr>());
    } else if (item.hasPayload<KCalCore::Todo::Ptr>()) {
        xapDoc = indexTodo(item.payload<KCalCore::Todo::Ptr>());
    } else if (item.hasPayload<KCalCore::Journal::Ptr>()) {
        xapDoc = indexJournal(item.payload<KCalCore::Journal::Ptr>());
    } else {
        qCWarning(AKONADISEARCH_LOG) << "Unknown payload!";
        return {};
    }

    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Akonadi::Search::CalenderIndexer::index",
               "Item does not have a valid parent collection");

    XapianDocument doc(xapDoc);
    const Akonadi::Collection::Id colId = item.parentCollection().id();
    doc.addCollectionTerm(colId);

    return doc.xapianDocument();
}

Xapian::Document IncidenceIndexer::indexEvent(const KCalCore::Event::Ptr& event)
{
    XapianDocument doc;

    doc.indexText(event->organizer()->email(), QStringLiteral("O"));
    doc.indexText(event->summary(), QStringLiteral("S"));
    doc.indexText(event->location(), QStringLiteral("L"));
    KCalCore::Attendee::List attendees = event->attendees();
    KCalCore::Attendee::List::ConstIterator it;
    KCalCore::Attendee::List::ConstIterator end(attendees.constEnd());
    for (it = attendees.constBegin(); it != end; ++it) {
        doc.addBoolTerm((*it)->email() + QString::number((*it)->status()), QStringLiteral("PS"));
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
