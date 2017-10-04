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

#include "incidencequerymapper.h"
#include "querymapper_p.h"
#include "akonadisearch_debug.h"

#include <KCalCore/Event>
#include <KCalCore/Todo>
#include <KCalCore/Journal>

#include <AkonadiCore/SearchQuery>

#include <QVariant>

using namespace Akonadi::Search;

IncidenceQueryMapper::IncidenceQueryMapper()
{
    mPropMapper.insertPrefix(QStringLiteral("organizer"), QStringLiteral("O"));
    mPropMapper.insertPrefix(QStringLiteral("partstatus"), QStringLiteral("PS"));
    mPropMapper.insertPrefix(QStringLiteral("summary"), QStringLiteral("S"));
    mPropMapper.insertPrefix(QStringLiteral("location"), QStringLiteral("L"));

    mPropMapper.insertBoolValueProperty(QStringLiteral("partstatus"));
}

QStringList IncidenceQueryMapper::mimeTypes()
{
    return { KCalCore::Event::eventMimeType(),
             KCalCore::Todo::todoMimeType(),
             KCalCore::Journal::journalMimeType() };
}

Xapian::Query IncidenceQueryMapper::recursiveTermMapping(const SearchTerm &term)
{
    const Akonadi::IncidenceSearchTerm::IncidenceSearchField field = Akonadi::IncidenceSearchTerm::fromKey(term.key());
    switch (field) {
    case Akonadi::IncidenceSearchTerm::Organizer:
        return constructQuery(mPropMapper, QStringLiteral("organizer"), term);
    case Akonadi::IncidenceSearchTerm::Summary:
        return constructQuery(mPropMapper, QStringLiteral("summary"), term);
    case Akonadi::IncidenceSearchTerm::Location:
        return constructQuery(mPropMapper, QStringLiteral("location"), term);
    case Akonadi::IncidenceSearchTerm::PartStatus: {
        return negateQuery(
            constructQuery(mPropMapper, QStringLiteral("partstatus"),
                            term.value().toString(), SearchTerm::CondEqual),
            term.isNegated());
    }
    default:
        return QueryMapper::recursiveTermMapping(term);
    }
}
