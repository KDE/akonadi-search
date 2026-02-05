/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "search_pim_export.h"
#include <QString>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
// FIXME: Make this async!!
/*!
 * \class Akonadi::Search::PIM::ContactCompleter
 * \inheader AkonadiSearch/PIM/ContactCompleter
 * \inmodule AkonadiSearchPIM
 * \brief Provides contact auto-completion functionality.
 *
 * ContactCompleter provides email address auto-completion based on a prefix
 * string, searching through indexed contacts.
 *
 */
class AKONADI_SEARCH_PIM_EXPORT ContactCompleter
{
public:
    /*!
     * \brief Constructs a contact completer.
     * \param prefix The prefix string to complete.
     * \param limit The maximum number of completions to return. Default is 10.
     */
    explicit ContactCompleter(const QString &prefix, int limit = 10);

    /*!
     * \brief Returns the completed contact list.
     * \return A list of email addresses matching the prefix.
     */
    [[nodiscard]] QStringList complete() const;

private:
    const QString m_prefix;
    const int m_limit;
};
}
}
}
