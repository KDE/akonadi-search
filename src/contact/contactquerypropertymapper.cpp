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

#include "contactquerypropertymapper.h"

#include <AkonadiCore/SearchQuery>

#include <QMutex>

using namespace Akonadi::Search;

ContactQueryPropertyMapper *ContactQueryPropertyMapper::sInstance = nullptr;

ContactQueryPropertyMapper::ContactQueryPropertyMapper()
{
    insertPrefix(Akonadi::ContactSearchTerm::Name, QStringLiteral("NA"));
    insertPrefix(Akonadi::ContactSearchTerm::Nickname, QStringLiteral("NI"));
    insertPrefix(Akonadi::ContactSearchTerm::Email, QStringLiteral("")); // Email currently doesn't map to anything
    insertPrefix(Akonadi::ContactSearchTerm::Uid, QStringLiteral("UID"));

    insertValueProperty(Akonadi::ContactSearchTerm::Birthday, 0);
    insertValueProperty(Akonadi::ContactSearchTerm::Anniversary, 1);
}

const ContactQueryPropertyMapper &ContactQueryPropertyMapper::instance()
{
    static QMutex lock;
    lock.lock();
    if (!sInstance) {
        sInstance = new ContactQueryPropertyMapper();
    }
    lock.unlock();
    return *sInstance;
}
