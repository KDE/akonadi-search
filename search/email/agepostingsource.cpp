/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "agepostingsource.h"

#include <QDateTime>
#include <QString>

#include <cmath>

using namespace Akonadi::Search;

AgePostingSource::AgePostingSource(Xapian::valueno slot_)
    : Xapian::ValuePostingSource(slot_)
    , m_currentTime_t(QDateTime::currentDateTimeUtc().toSecsSinceEpoch())
{
}

Xapian::weight AgePostingSource::get_weight() const
{
    const std::string s = *value_it;
    const QString str = QString::fromUtf8(s.c_str(), s.length());

    bool ok = false;
    const uint time = str.toUInt(&ok);

    if (!ok) {
        return 0.0;
    }

    const uint diff = m_currentTime_t - time;

    // Each day is given a penalty of penalty of 1.0
    const double penalty = 1.0 / (24 * 60 * 60);
    const double result = 1000.0 - (diff * penalty);

    if (result < 0.0) {
        return 0.0;
    }

    return result;
}

Xapian::PostingSource *AgePostingSource::clone() const
{
    return new AgePostingSource(slot);
}

void AgePostingSource::init(const Xapian::Database &db_)
{
    Xapian::ValuePostingSource::init(db_);
    set_maxweight(1000.0);
}
