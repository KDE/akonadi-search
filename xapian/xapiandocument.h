/*
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include <QString>
#include <xapian.h>

#include "search_xapian_export.h"
#include "xapiantermgenerator.h"

namespace Akonadi
{
namespace Search
{
/**
 * This class is just a light wrapper over Xapian::Document
 * which provides nice Qt apis.
 */
class AKONADI_SEARCH_XAPIAN_EXPORT XapianDocument
{
public:
    XapianDocument();
    XapianDocument(const Xapian::Document &doc);

    void addTerm(const QString &term, const QString &prefix = QString());
    void addBoolTerm(const QString &term, const QString &prefix = QString());
    void addBoolTerm(int term, const QString &prefix);

    void indexText(const QString &text, int wdfInc = 1);
    void indexText(const QString &text, const QString &prefix, int wdfInc = 1);

    void addValue(int pos, const QString &value);

    Xapian::Document doc() const;

    QString fetchTermStartsWith(const QByteArray &term);

    /**
     * Remove all the terms which start with the prefix \p prefix
     *
     * \return true if the document was modified
     */
    bool removeTermStartsWith(const QByteArray &prefix);

private:
    Xapian::Document m_doc;
    XapianTermGenerator m_termGen;
};
}
}

