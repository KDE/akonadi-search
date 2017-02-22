/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2014 Laurent Montel <montel@kde.org>
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

#include "calendarindexer.h"
#include "xapiandocument.h"
#include "akonadi_indexer_agent_debug.h"

#include <KCalCore/Attendee>
#include <KCalCore/Event>

#include <QTextDocument>

CalendarIndexer::CalendarIndexer(const QString &path)
    : AbstractIndexer(), m_db(nullptr)
{
    try {
        m_db = new Akonadi::Search::XapianDatabase(path, true);
    } catch (const Xapian::DatabaseCorruptError &err) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Database Corrupted - What did you do?";
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << err.get_error_string();
        m_db = nullptr;
    } catch (const Xapian::Error &e) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << QString::fromStdString(e.get_type()) << QString::fromStdString(e.get_description());
        m_db = nullptr;
    }
}

CalendarIndexer::~CalendarIndexer()
{
    if (m_db) {
        m_db->commit();
        delete m_db;
    }
}

QStringList CalendarIndexer::mimeTypes() const
{
    return QStringList() << QString::fromLatin1("application/x-vnd.akonadi.calendar.event")
           << QString::fromLatin1("application/x-vnd.akonadi.calendar.todo")
           << QString::fromLatin1("application/x-vnd.akonadi.calendar.journal")
           << QString::fromLatin1("application/x-vnd.akonadi.calendar.freebusy");
}

void CalendarIndexer::index(const Akonadi::Item &item)
{
    if (item.hasPayload<KCalCore::Event::Ptr>()) {
        indexEventItem(item, item.payload<KCalCore::Event::Ptr>());
    } else if (item.hasPayload<KCalCore::Journal::Ptr>()) {
        indexJournalItem(item, item.payload<KCalCore::Journal::Ptr>());
    } else if (item.hasPayload<KCalCore::Todo::Ptr>()) {
        indexTodoItem(item, item.payload<KCalCore::Todo::Ptr>());
    } else {
        return;
    }
}

void CalendarIndexer::commit()
{
    if (m_db) {
        m_db->commit();
    }
}

void CalendarIndexer::remove(const Akonadi::Item &item)
{
    if (!m_db) {
        return;
    }
    try {
        m_db->deleteDocument(item.id());
    } catch (const Xapian::DocNotFoundError &) {
        return;
    }
}

void CalendarIndexer::remove(const Akonadi::Collection &collection)
{
    if (!m_db) {
        return;
    }
    try {
        Xapian::Query query('C' + QString::number(collection.id()).toStdString());
        Xapian::Enquire enquire(*(m_db->db()));
        enquire.set_query(query);

        Xapian::MSet mset = enquire.get_mset(0, m_db->db()->get_doccount());
        Xapian::MSetIterator end(mset.end());
        for (Xapian::MSetIterator it = mset.begin(); it != end; ++it) {
            const qint64 id = *it;
            remove(Akonadi::Item(id));
        }
    } catch (const Xapian::DocNotFoundError &) {
        return;
    }
}

void CalendarIndexer::move(Akonadi::Item::Id itemId,
                           Akonadi::Collection::Id from,
                           Akonadi::Collection::Id to)
{
    if (!m_db) {
        return;
    }
    Xapian::Document doc;
    try {
        doc = m_db->db()->get_document(itemId);
    } catch (const Xapian::DocNotFoundError &) {
        return;
    }

    const QByteArray ft = 'C' + QByteArray::number(from);
    const QByteArray tt = 'C' + QByteArray::number(to);

    doc.remove_term(ft.data());
    doc.add_boolean_term(tt.data());
    m_db->replaceDocument(doc.get_docid(), doc);
}

void CalendarIndexer::indexEventItem(const Akonadi::Item &item, const KCalCore::Event::Ptr &event)
{
    Akonadi::Search::XapianDocument doc;

    doc.indexText(event->organizer()->email(), "O");
    doc.indexText(event->summary(), "S");
    doc.indexText(event->location(), "L");
    KCalCore::Attendee::List attendees = event->attendees();
    KCalCore::Attendee::List::ConstIterator it;
    KCalCore::Attendee::List::ConstIterator end(attendees.constEnd());
    for (it = attendees.constBegin(); it != end; ++it) {
        doc.addBoolTerm((*it)->email() + QString::number((*it)->status()), "PS");
    }

    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Akonadi::Search::CalenderIndexer::index",
               "Item does not have a valid parent collection");

    const Akonadi::Collection::Id colId = item.parentCollection().id();
    doc.addBoolTerm(colId, "C");

    m_db->replaceDocument(item.id(), doc);
}

void CalendarIndexer::indexJournalItem(const Akonadi::Item &item, const KCalCore::Journal::Ptr &journal)
{
    //TODO
    Q_UNUSED(item);
    Q_UNUSED(journal);
}

void CalendarIndexer::indexTodoItem(const Akonadi::Item &item, const KCalCore::Todo::Ptr &todo)
{
    //TODO
    Q_UNUSED(item);
    Q_UNUSED(todo);
}

void CalendarIndexer::updateIncidenceItem(const KCalCore::Incidence::Ptr &calInc)
{
    //TODO
    Q_UNUSED(calInc);
}
