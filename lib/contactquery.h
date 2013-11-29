/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CONTACTQUERY_H
#define CONTACTQUERY_H

#include <QString>
#include "query.h"

namespace Baloo {
namespace PIM {

/**
 * Query for a list of contacts matching a criteria
 */
class BALOO_PIM_EXPORT ContactQuery : public Query
{
public:
    ContactQuery();
    ~ContactQuery();

    void matchName(const QString& name);
    void matchNickname(const QString& nick);
    void matchEmail(const QString& email);
    void matchUID(const QString& uid);
    void match(const QString& str);

    enum MatchCriteria {
        ExactMatch,
        StartsWithMatch
    };

    void setMatchCriteria(MatchCriteria m);
    MatchCriteria matchCriteria() const;

    ResultIterator exec();

    int limit() const;
    void setLimit(int limit);

private:
    class Private;
    Private* d;
};

}
}

#endif // CONTACTQUERY_H
