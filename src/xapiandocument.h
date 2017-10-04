/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef AKONADI_SEARCH_XAPIANDOCUMENT_H
#define AKONADI_SEARCH_XAPIANDOCUMENT_H

#include "akonadisearch_export.h"

#include <QString>

namespace Xapian {
class Document;
}

namespace Akonadi
{
namespace Search
{


class XapianDocumentPrivate;

/**
 * This class is just a light wrapper over Xapian::Document
 * which provides nice Qt apis.
 */
class AKONADISEARCH_EXPORT XapianDocument
{
public:
    explicit XapianDocument();
    explicit XapianDocument(const Xapian::Document &doc);
    ~XapianDocument();

    void addCollectionTerm(qint64 colId);

    void addTerm(const QString &term, const QString &prefix = QString());
    void addBoolTerm(const QString &term, const QString &prefix = QString());
    void addBoolTerm(int term, const QString &prefix);

    void indexText(const QString &text, int wdfInc = 1);
    void indexText(const QString &text, const QString &prefix, int wdfInc = 1);
    void indexTextWithoutPositions(const QString &text, const QString &prefix = {}, int wfdInc = 1);
    void setData(const QString &data);

    void addValue(int pos, const QString &value);
    void addValue(int pos, double value);

    Xapian::Document xapianDocument() const;

    QString fetchTermStartsWith(const QByteArray &term);

    /**
     * Remove all the terms which start with the prefix \p prefix
     *
     * \return true if the document was modified
     */
    bool removeTermStartsWith(const QByteArray &prefix);
    bool removeTerm(const QByteArray &term);

    static QByteArray collectionId(qint64 id);

private:
    XapianDocumentPrivate * const d;

};
}
}

#endif // AKONADI_SEARCH_XAPIANDOCUMENT_H
