/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "query.h"
#include "search_pim_export.h"

#include <QString>

#include <memory>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
class ContactQueryPrivate;

/**
 * Query for a list of contacts matching a criteria
 */
class AKONADI_SEARCH_PIM_EXPORT ContactQuery : public Query
{
public:
    ContactQuery();
    ~ContactQuery() override;

    void matchName(const QString &name);
    void matchNickname(const QString &nick);
    void matchEmail(const QString &email);
    void matchUID(const QString &uid);
    void match(const QString &str);

    enum MatchCriteria {
        ExactMatch,
        StartsWithMatch,
    };

    void setMatchCriteria(MatchCriteria m);
    Q_REQUIRED_RESULT MatchCriteria matchCriteria() const;

    ResultIterator exec() override;

    Q_REQUIRED_RESULT int limit() const;
    void setLimit(int limit);

private:
    std::unique_ptr<ContactQueryPrivate> const d;
};
}
}
}
