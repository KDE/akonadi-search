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

#include "contactquerymapper.h"
#include "querymapper_p.h"
#include "akonadisearch_debug.h"

#include <AkonadiCore/SearchQuery>

#include <KContacts/Addressee>
#include <KContacts/ContactGroup>

using namespace Akonadi::Search;

ContactQueryMapper::ContactQueryMapper()
{
    mPropMapper.insertPrefix(QStringLiteral("name"), QStringLiteral("NA"));
    mPropMapper.insertPrefix(QStringLiteral("nick"), QStringLiteral("NI"));
    mPropMapper.insertPrefix(QStringLiteral("email"), QStringLiteral("")); // Email currently doesn't map to anything
    mPropMapper.insertPrefix(QStringLiteral("uid"), QStringLiteral("UID"));

    mPropMapper.insertValueProperty(QStringLiteral("birthday"), 0);
    mPropMapper.insertValueProperty(QStringLiteral("anniversary"), 1);
}

QStringList ContactQueryMapper::mimeTypes()
{
    return { KContacts::Addressee::mimeType(),
             KContacts::ContactGroup::mimeType() };
}

Xapian::Query ContactQueryMapper::recursiveTermMapping(const Akonadi::SearchTerm &term)
{
    const Akonadi::ContactSearchTerm::ContactSearchField field = Akonadi::ContactSearchTerm::fromKey(term.key());
    switch (field) {
    case Akonadi::ContactSearchTerm::Name:
        return constructQuery(mPropMapper, QStringLiteral("name"), term);
    case Akonadi::ContactSearchTerm::Email:
        return constructQuery(mPropMapper, QStringLiteral("email"), term);
    case Akonadi::ContactSearchTerm::Nickname:
        return constructQuery(mPropMapper, QStringLiteral("nick"), term);
    case Akonadi::ContactSearchTerm::Uid:
        return constructQuery(mPropMapper, QStringLiteral("uid"), term);
    case Akonadi::ContactSearchTerm::Unknown:
    default:
        return QueryMapper::recursiveTermMapping(term);
    }
}
