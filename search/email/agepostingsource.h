/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#ifndef AKONADI_SEARCH_PIM_AGEPOSTINGSOURCE_H
#define AKONADI_SEARCH_PIM_AGEPOSTINGSOURCE_H

#include <xapian.h>

namespace Akonadi {
namespace Search {
class AgePostingSource : public Xapian::ValuePostingSource
{
public:
    explicit AgePostingSource(Xapian::valueno slot_);

    Xapian::weight get_weight() const override;
    Xapian::PostingSource *clone() const override;

    std::string name() const override
    {
        return "AgePostingSource";
    }

    void init(const Xapian::Database &db_) override;

private:
    const unsigned int m_currentTime_t;
};
}
}

#endif // AKONADI_SEARCH_PIM_AGEPOSTINGSOURCE_H
