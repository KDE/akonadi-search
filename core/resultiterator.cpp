/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "resultiterator.h"

using namespace Akonadi::Search;

ResultIterator::ResultIterator(int id, SearchStore *store)
    : d(new ResultIteratorPrivate)
{
    d->queryId = id;
    d->store = store;
}

ResultIterator::ResultIterator()
    : d(new ResultIteratorPrivate)
{
}

ResultIterator::ResultIterator(const ResultIterator &rhs) = default;

ResultIterator::~ResultIterator() = default;

ResultIterator &ResultIterator::operator=(const ResultIterator &other) = default;

bool ResultIterator::next()
{
    if (d->store) {
        return d->store->next(d->queryId);
    } else {
        return false;
    }
}

QByteArray ResultIterator::id() const
{
    if (d->store) {
        return d->store->id(d->queryId);
    } else {
        return {};
    }
}

QUrl ResultIterator::url() const
{
    if (d->store) {
        return d->store->url(d->queryId);
    } else {
        return {};
    }
}

QString ResultIterator::text() const
{
    if (d->store) {
        return d->store->text(d->queryId);
    } else {
        return {};
    }
}

QString ResultIterator::icon() const
{
    if (d->store) {
        return d->store->icon(d->queryId);
    } else {
        return {};
    }
}
