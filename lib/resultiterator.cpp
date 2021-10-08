/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "resultiterator_p.h"

using namespace Akonadi::Search::PIM;

ResultIterator::ResultIterator()
    : d(new Private)
{
}

ResultIterator::ResultIterator(const ResultIterator &ri)
    : d(new Private(*ri.d))
{
}

ResultIterator::~ResultIterator() = default;

bool ResultIterator::next()
{
    if (d->m_iter == d->m_end) {
        return false;
    }

    if (d->m_firstElement) {
        d->m_iter = d->m_mset.begin();
        d->m_firstElement = false;
        return d->m_iter != d->m_end;
    }

    ++d->m_iter;
    return d->m_iter != d->m_end;
}

Akonadi::Item::Id ResultIterator::id()
{
    // qDebug() << d->m_iter.get_rank() << d->m_iter.get_weight();
    return *(d->m_iter);
}
