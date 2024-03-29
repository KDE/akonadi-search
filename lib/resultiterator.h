/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "search_pim_export.h"

#include <Akonadi/Item>

#include <memory>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
class ContactQuery;
class EmailQuery;
class NoteQuery;
class ResultIteratorPrivate;

/** Result iterator. */
class AKONADI_SEARCH_PIM_EXPORT ResultIterator
{
public:
    ResultIterator();
    ResultIterator(const ResultIterator &ri);
    ~ResultIterator();

    [[nodiscard]] Akonadi::Item::Id id();
    bool next();

private:
    friend class ContactQuery;
    friend class EmailQuery;
    friend class NoteQuery;
    friend class CollectionQuery;

    std::unique_ptr<ResultIteratorPrivate> const d;
};
}
}
}
