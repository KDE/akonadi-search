/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2014-2020 Laurent Montel <montel@kde.org>
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
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
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
    ~CalendarIndexer();

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
