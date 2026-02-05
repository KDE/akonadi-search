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

/*!
 * \class Akonadi::Search::PIM::ContactQuery
 * \inheader AkonadiSearch/PIM/ContactQuery
 * \inmodule AkonadiSearchPIM
 * \brief Search query for contacts.
 *
 * ContactQuery allows searching for contacts by name, nickname, email,
 * UID, and other criteria with configurable match types.
 *
 * \sa Query, ResultIterator
 */
class AKONADI_SEARCH_PIM_EXPORT ContactQuery : public Query
{
public:
    /*!
     * \enum ContactQuery::MatchCriteria
     * \brief Defines how contact fields are matched.
     */
    enum MatchCriteria : uint8_t {
        ExactMatch, /*!< Exact match. */
        StartsWithMatch, /*!< Starts with match. */
    };

    /*!
     * \brief Constructs an empty contact query.
     */
    ContactQuery();
    /*!
     * \brief Destructs the contact query.
     */
    ~ContactQuery() override;

private:
    std::unique_ptr<ContactQueryPrivate> const d;
};
}
}
}
