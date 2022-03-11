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
/** Xapian term generator. */
class AKONADI_SEARCH_XAPIAN_EXPORT XapianTermGenerator
{
public:
    explicit XapianTermGenerator(Xapian::Document *doc);

    void indexText(const QString &text);
    void indexText(const QString &text, const QString &prefix, int wdfInc = 1);

    void setPosition(int position);
    Q_REQUIRED_RESULT int position() const;

    void setDocument(Xapian::Document *doc);

    static QStringList termList(const QString &text);

private:
    Xapian::Document *m_doc = nullptr;
    Xapian::TermGenerator m_termGen;

    int m_position = 1;
};
}
}

