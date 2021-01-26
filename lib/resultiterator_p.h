/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#ifndef AKONADI_SEARCH_PIM_RESULTITERATOR_P_H
#define AKONADI_SEARCH_PIM_RESULTITERATOR_P_H

#include <xapian.h>

#include "resultiterator.h"

namespace Akonadi
{
namespace Search
{
namespace PIM
{
class Q_DECL_HIDDEN ResultIterator::Private
{
public:
    void init(const Xapian::MSet &mset)
    {
        m_mset = mset;
        m_end = m_mset.end();
        m_iter = m_mset.begin();
        m_firstElement = true;
    }

    Xapian::MSet m_mset;
    Xapian::MSetIterator m_iter;
    Xapian::MSetIterator m_end;

    bool m_firstElement = false;
};
}
}
}

#endif // AKONADI_SEARCH_PIM_RESULTITERATOR_P_H
