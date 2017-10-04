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

#include <xapian.h>

#include "querymapper.h"
#include "querymapper_p.h"
#include "querypropertymapper_p.h"
#include "registrar_p.h"
#include "akonadisearch_debug.h"

#include "email/emailquerymapper.h"
#include "contact/contactquerymapper.h"
#include "incidence/incidencequerymapper.h"
#include "note/notequerymapper.h"

#include <AkonadiCore/SearchQuery>

using namespace Akonadi::Search;

namespace {

Q_GLOBAL_STATIC(Registrar<QueryMapper>, sQueryMappers)

}

QueryMapper::QueryMapper()
{
}

QueryMapper::~QueryMapper()
{
}

Xapian::Query QueryMapper::map(const Akonadi::SearchQuery &akQuery)
{
    try {
        return recursiveTermMapping(akQuery.term());
    } catch (const Xapian::UnimplementedError &e) {
        qCWarning(AKONADISEARCH_LOG) << "Unsupported Xapian query:" << e.get_description().c_str();
        return {};
    } catch (const Xapian::QueryParserError &e) {
        qCWarning(AKONADISEARCH_LOG) << "Query Parser error:" << e.get_description().c_str();
        return {};
    }
}


Xapian::Query QueryMapper::recursiveTermMapping(const Akonadi::SearchTerm &term)
{
    if (!term.subTerms().isEmpty()) {
        QVector<Xapian::Query> sub;
        const auto subTerms = term.subTerms();
        for (const auto &t : subTerms) {
            const auto q = recursiveTermMapping(t);
            if (!q.empty()) {
                sub.push_back(q);
            }
        }
        return Xapian::Query{ mapRelation(term.relation()), sub.cbegin(), sub.cend() };
    } else {
        const auto field = Akonadi::SearchTerm::fromKey(term.key());
        switch (field) {
        case Akonadi::SearchTerm::Collection:
            return constructQuery(propertyMapper(), QStringLiteral("collection"), term);
        default:
            qCWarning(AKONADISEARCH_LOG) << "unknown term " << term.key();
        }
    }

    return {};
}


QVector<QueryMapper*> QueryMapper::forType(const QString &mimeType)
{
    if (!sQueryMappers.exists()) {
        sQueryMappers->registerForType<EmailQueryMapper>();
        sQueryMappers->registerForType<ContactQueryMapper>();
        sQueryMappers->registerForType<IncidenceQueryMapper>();
        sQueryMappers->registerForType<NoteQueryMapper>();
    }

    return sQueryMappers->spawnInstancesForType(mimeType);
}
