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

#include "akonadisearch_debug.h"
#include "querymapper_p.h"
#include "querypropertymapper_p.h"

#include <QVariant>
#include <QByteArray>

#include <QDebug>

namespace Akonadi {
namespace Search {

Xapian::Query::op mapRelation(SearchTerm::Relation rel)
{
    switch (rel) {
    case SearchTerm::RelAnd: return Xapian::Query::OP_AND;
    case SearchTerm::RelOr: return Xapian::Query::OP_OR;
    }
    Q_UNREACHABLE();
}

Xapian::Query negateQuery(const Xapian::Query &query, bool negate)
{
    if (negate) {
        return Xapian::Query(Xapian::Query::OP_AND_NOT, Xapian::Query::MatchAll, query);
    } else {
        return query;
    }
}

Xapian::Query constructQuery(const QueryPropertyMapper &mapper,
                             int propertyKey, const QVariant &value,
                             SearchTerm::Condition cond)
{
    if (value.isNull()) {
        return {};
    }
    if (mapper.hasBoolProperty(propertyKey)) {
        const auto p = mapper.prefix(propertyKey);
        if (p.empty()) {
            return {};
        }

        std::string term("B");
        bool isTrue = false;

        if (value.isNull()) {
            isTrue = true;
        }

        if (value.type() == QVariant::Bool) {
            isTrue = value.toBool();
        }

        if (isTrue) {
            term += p;
        } else {
            term += 'N' + p;
        }
        return Xapian::Query(term);
    } else if (mapper.hasBoolValueProperty(propertyKey)) {
        const auto term = mapper.prefix(propertyKey);
        std::string val(value.toString().toStdString());
        return Xapian::Query(term + val);
    } else if (mapper.hasValueProperty(propertyKey)
            && (cond == SearchTerm::CondEqual
                || cond == SearchTerm::CondGreaterThan || cond == SearchTerm::CondGreaterOrEqual 
                || cond == SearchTerm::CondLessThan || cond == SearchTerm::CondLessOrEqual)) {
        auto numVal = value.toLongLong();
        if (cond == SearchTerm::CondGreaterThan) {
            ++numVal;
        }
        if (cond == SearchTerm::CondLessThan) {
            --numVal;
        }
        const int valueNumber = mapper.valueProperty(propertyKey);
        if (cond == SearchTerm::CondGreaterOrEqual || cond == SearchTerm::CondGreaterThan) {
            return Xapian::Query(Xapian::Query::OP_VALUE_GE, valueNumber, Xapian::sortable_serialise(numVal));
        } else if (cond == SearchTerm::CondLessOrEqual || cond == SearchTerm::CondLessThan) {
            return Xapian::Query(Xapian::Query::OP_VALUE_LE, valueNumber, Xapian::sortable_serialise(numVal));
        } else if (cond == SearchTerm::CondEqual) {
            const auto serialisedVal = Xapian::sortable_serialise(numVal);
            return Xapian::Query(Xapian::Query::OP_VALUE_RANGE, valueNumber, serialisedVal, serialisedVal);
        }
    } else if ((cond == SearchTerm::CondContains || cond == SearchTerm::CondEqual)
                && mapper.hasPrefix(propertyKey)) {
        const auto prefix = mapper.prefix(propertyKey);
        std::string str = value.toString().toStdString();
        Xapian::QueryParser parser;
        parser.set_stemming_strategy(Xapian::QueryParser::STEM_NONE);
        int flags = Xapian::QueryParser::FLAG_DEFAULT;
        if (cond == SearchTerm::CondContains) {
            flags |= Xapian::QueryParser::FLAG_PARTIAL;
        } else {
            flags |= Xapian::QueryParser::FLAG_PHRASE;
            str = "\"" + str + "\"";
        }

        auto q = parser.parse_query(str, flags, prefix);
        if (cond == SearchTerm::CondEqual) {
            QVector<Xapian::Query> v;
            v.push_back(Xapian::Query(prefix + "^", 1, 1));
            const int subqueries = q.get_num_subqueries();
            if (subqueries > 0) {
                // FIXME This is still not perfect as the subquries will have termpos counted from 1
                // but it's the best we can do to get the results we want.
                for (int i = 0; i < subqueries; ++i) {
                    auto sq = q.get_subquery(i);
                    v.push_back(sq);
                }
            } else {
                v.push_back(q);
            }
            v.push_back(Xapian::Query(prefix + "$", 1, subqueries + 2));
            q = Xapian::Query(Xapian::Query::OP_PHRASE, v.begin(), v.end());
        }
        //qCDebug(AKONADISEARCH_LOG) << q.get_description().c_str();
        return q;
    }

    return {};
}

Xapian::Query constructQuery(const QueryPropertyMapper &mapper,
                             int propertyKey,
                             const SearchTerm &term)
{
    return negateQuery(
                constructQuery(mapper, propertyKey, term.value(), term.condition()), 
                term.isNegated());
}

Xapian::Query constructQuery(const QueryPropertyMapper &mapper,
                             int propertyKey,
                             bool val)
{
    return constructQuery(mapper, propertyKey, val, SearchTerm::CondEqual);
}

}
}
