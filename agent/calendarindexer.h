/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2020 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#ifndef CALENDARINDEXER_H
#define CALENDARINDEXER_H

#include "abstractindexer.h"
#include "xapiandatabase.h"

#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#include <KCalendarCore/Journal>
#include <KCalendarCore/Event>
#include <KCalendarCore/Todo>

class CalendarIndexer : public AbstractIndexer
{
public:
    /**
     * You must provide the path where the indexed information
     * should be stored
     */
    explicit CalendarIndexer(const QString &path);
    ~CalendarIndexer() override;

    QStringList mimeTypes() const override;

    void index(const Akonadi::Item &item) override;
    void commit() override;

    void remove(const Akonadi::Item &item) override;
    void remove(const Akonadi::Collection &collection) override;
    void move(Akonadi::Item::Id itemId, Akonadi::Collection::Id from, Akonadi::Collection::Id to) override;
private:
    void indexEventItem(const Akonadi::Item &item, const KCalendarCore::Event::Ptr &event);
    void indexJournalItem(const Akonadi::Item &item, const KCalendarCore::Journal::Ptr &journal);
    void indexTodoItem(const Akonadi::Item &item, const KCalendarCore::Todo::Ptr &todo);
    void updateIncidenceItem(const KCalendarCore::Incidence::Ptr &calInc);

    Akonadi::Search::XapianDatabase *m_db = nullptr;
};

#endif // CALENDARINDEXER_H
