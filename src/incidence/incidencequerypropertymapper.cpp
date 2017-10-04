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

#include "incidencequerypropertymapper.h"

#include <AkonadiCore/SearchQuery>

#include <QMutex>

using namespace Akonadi::Search;

IncidenceQueryPropertyMapper *IncidenceQueryPropertyMapper::sInstance = nullptr;

IncidenceQueryPropertyMapper::IncidenceQueryPropertyMapper()
{
    insertPrefix(Akonadi::IncidenceSearchTerm::Organizer, QStringLiteral("O"));
    insertPrefix(Akonadi::IncidenceSearchTerm::PartStatus, QStringLiteral("PS"));
    insertPrefix(Akonadi::IncidenceSearchTerm::Summary, QStringLiteral("S"));
    insertPrefix(Akonadi::IncidenceSearchTerm::Location, QStringLiteral("L"));

    insertBoolValueProperty(Akonadi::IncidenceSearchTerm::PartStatus);
}

const IncidenceQueryPropertyMapper &IncidenceQueryPropertyMapper::instance()
{
    static QMutex lock;
    lock.lock();
    if (!sInstance) {
        sInstance = new IncidenceQueryPropertyMapper();
    }
    lock.unlock();
    return *sInstance;
}

