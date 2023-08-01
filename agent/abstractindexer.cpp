/*
 * SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "abstractindexer.h"
#include <TextUtils/ConvertText>

AbstractIndexer::AbstractIndexer() = default;

AbstractIndexer::~AbstractIndexer() = default;

void AbstractIndexer::move(Akonadi::Item::Id item, Akonadi::Collection::Id from, Akonadi::Collection::Id to)
{
    Q_UNUSED(item)
    Q_UNUSED(from)
    Q_UNUSED(to)
}

void AbstractIndexer::updateFlags(const Akonadi::Item &item, const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removed)
{
    Q_UNUSED(item)
    Q_UNUSED(addedFlags)
    Q_UNUSED(removed)
}

bool AbstractIndexer::respectDiacriticAndAccents() const
{
    return mRespectDiacriticAndAccents;
}

void AbstractIndexer::setRespectDiacriticAndAccents(bool newRespectDiacriticAndAccents)
{
    mRespectDiacriticAndAccents = newRespectDiacriticAndAccents;
}

QString AbstractIndexer::normalizeString(const QString &str)
{
    if (mRespectDiacriticAndAccents) {
        return str;
    } else {
        return TextUtils::ConvertText::normalize(str);
    }
}
