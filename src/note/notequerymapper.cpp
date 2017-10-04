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

#include "notequerymapper.h"
#include "querymapper_p.h"
#include "akonadisearch_debug.h"

#include <AkonadiCore/SearchQuery>

using namespace Akonadi::Search;

NoteQueryMapper::NoteQueryMapper()
{
    mPropMapper.insertPrefix(QStringLiteral("subject"), QStringLiteral("SU"));
    mPropMapper.insertPrefix(QStringLiteral("body"), QStringLiteral("BO"));
}

QStringList NoteQueryMapper::mimeTypes()
{
    return { QStringLiteral("text/x-vnd.akonadi.note") };
}

Xapian::Query NoteQueryMapper::recursiveTermMapping(const Akonadi::SearchTerm &term)
{
    // qCDebug(AKONADIPLUGIN_INDEXER_LOG) << term.key() << term.value();
    const Akonadi::EmailSearchTerm::EmailSearchField field = Akonadi::EmailSearchTerm::fromKey(term.key());
    switch (field) {
    case Akonadi::EmailSearchTerm::Subject:
        return constructQuery(mPropMapper, QStringLiteral("subject"), term);
    case Akonadi::EmailSearchTerm::Body:
        return constructQuery(mPropMapper, QStringLiteral("body"), term);
    default:
        return QueryMapper::recursiveTermMapping(term);
    }
}
