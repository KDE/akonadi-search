/*
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include <xapian.h>

#include "search_xapian_export.h"
#include <QString>

namespace Akonadi
{
namespace Search
{
/*! Xapian query parser. */
class AKONADI_SEARCH_XAPIAN_EXPORT XapianQueryParser
{
public:
    /*!
     */
    XapianQueryParser();

    /*!
     */
    void setDatabase(Xapian::Database *db);
    /*!
     */
    [[nodiscard]] Xapian::Query parseQuery(const QString &str, const QString &prefix = QString());

    /*!
     * Expands word to every possible option which it can be expanded to.
     */
    [[nodiscard]] Xapian::Query expandWord(const QString &word, const QString &prefix = QString());

    /*!
     * Set if each word in the string should be treated as a partial word
     * and should be expanded to every possible word.
     */
    void setAutoExapand(bool autoexpand);

private:
    Xapian::Database *m_db = nullptr;
    bool m_autoExpand = true;
};
}
}
