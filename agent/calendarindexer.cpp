/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2021 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "calendarindexer.h"
#include "akonadi_indexer_agent_debug.h"
#include "xapiandocument.h"

#include <KCalendarCore/Attendee>

CalendarIndexer::CalendarIndexer(const QString &path)
    : AbstractIndexer()
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
    commit();
    delete m_db;
}

QStringList CalendarIndexer::mimeTypes() const
{
    return QStringList() << QStringLiteral("application/x-vnd.akonadi.calendar.event") << QStringLiteral("application/x-vnd.akonadi.calendar.todo")
                         << QStringLiteral("application/x-vnd.akonadi.calendar.journal") << QStringLiteral("application/x-vnd.akonadi.calendar.freebusy");
}

void CalendarIndexer::index(const Akonadi::Item &item)
{
    if (item.hasPayload<KCalendarCore::Event::Ptr>()) {
        indexEventItem(item, item.payload<KCalendarCore::Event::Ptr>());
    } else if (item.hasPayload<KCalendarCore::Journal::Ptr>()) {
        indexJournalItem(item, item.payload<KCalendarCore::Journal::Ptr>());
    } else if (item.hasPayload<KCalendarCore::Todo::Ptr>()) {
        indexTodoItem(item, item.payload<KCalendarCore::Todo::Ptr>());
    } else {
        return;
    }
}

void CalendarIndexer::commit()
{
    if (!m_db) {
        return;
    }

    try {
        m_db->commit();
    } catch (const Xapian::Error &err) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << err.get_error_string();
    }
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Xapian Committed";
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
        const Xapian::Query query('C' + QString::number(collection.id()).toStdString());
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

void CalendarIndexer::move(Akonadi::Item::Id itemId, Akonadi::Collection::Id from, Akonadi::Collection::Id to)
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

void CalendarIndexer::indexEventItem(const Akonadi::Item &item, const KCalendarCore::Event::Ptr &event)
{
    Akonadi::Search::XapianDocument doc;

    doc.indexText(event->organizer().email(), QStringLiteral("O"));
    doc.indexText(event->summary(), QStringLiteral("S"));
    doc.indexText(event->location(), QStringLiteral("L"));
    const KCalendarCore::Attendee::List attendees = event->attendees();
    KCalendarCore::Attendee::List::ConstIterator it;
    KCalendarCore::Attendee::List::ConstIterator end(attendees.constEnd());
    for (it = attendees.constBegin(); it != end; ++it) {
        doc.addBoolTerm((*it).email() + QString::number((*it).status()), QStringLiteral("PS"));
    }

    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Akonadi::Search::CalenderIndexer::index", "Item does not have a valid parent collection");

    const Akonadi::Collection::Id colId = item.parentCollection().id();
    doc.addBoolTerm(colId, QStringLiteral("C"));

    m_db->replaceDocument(item.id(), doc);
}

void CalendarIndexer::indexJournalItem(const Akonadi::Item &item, const KCalendarCore::Journal::Ptr &journal)
{
    // TODO
    Q_UNUSED(item)
    Q_UNUSED(journal)
}

void CalendarIndexer::indexTodoItem(const Akonadi::Item &item, const KCalendarCore::Todo::Ptr &todo)
{
    // TODO
    Q_UNUSED(item)
    Q_UNUSED(todo)
}

void CalendarIndexer::updateIncidenceItem(const KCalendarCore::Incidence::Ptr &calInc)
{
    // TODO
    Q_UNUSED(calInc)
}
