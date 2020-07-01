/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#ifndef AKONADI_SEARCH_PIM_CONTACTQUERY_H
#define AKONADI_SEARCH_PIM_CONTACTQUERY_H

#include "search_pim_export.h"
#include "query.h"

#include <QString>

namespace Akonadi {
namespace Search {
namespace PIM {
/**
 * Query for a list of contacts matching a criteria
 */
class AKONADI_SEARCH_PIM_EXPORT ContactQuery : public Query
{
public:
    ContactQuery();
    ~ContactQuery();

    void matchName(const QString &name);
    void matchNickname(const QString &nick);
    void matchEmail(const QString &email);
    void matchUID(const QString &uid);
    void match(const QString &str);

    enum MatchCriteria {
        ExactMatch,
        StartsWithMatch
    };

    void setMatchCriteria(MatchCriteria m);
    MatchCriteria matchCriteria() const;

    ResultIterator exec() override;

    int limit() const;
    void setLimit(int limit);

private:
    class Private;
    Private *d;
};
}
}
}

#endif // AKONADI_SEARCH_PIM_CONTACTQUERY_H
