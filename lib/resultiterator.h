/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#ifndef AKONADI_SEARCH_PIM_RESULT_ITERATOR_H
#define AKONADI_SEARCH_PIM_RESULT_ITERATOR_H

#include "search_pim_export.h"

#include <AkonadiCore/Item>

namespace Akonadi {
namespace Search {
namespace PIM {
class ContactQuery;
class EmailQuery;
class NoteQuery;

/** Result iterator. */
class AKONADI_SEARCH_PIM_EXPORT ResultIterator
{
public:
    ResultIterator();
    ResultIterator(const ResultIterator &ri);
    ~ResultIterator();

    Akonadi::Item::Id id();
    bool next();

private:
    friend class ContactQuery;
    friend class EmailQuery;
    friend class NoteQuery;
    friend class CollectionQuery;

    class Private;
    Private *const d;
};
}
}
}

#endif // AKONADI_SEARCH_PIM_RESULT_ITERATOR_H
