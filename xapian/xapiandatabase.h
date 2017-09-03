/*
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

#ifndef AKONADI_SEARCH_XAPIANDATABASE_H
#define AKONADI_SEARCH_XAPIANDATABASE_H

#include <xapian.h>
#include "search_xapian_export.h"

#include <QString>
#include <QPair>
#include <QVector>

namespace Akonadi
{
namespace Search
{

class XapianDocument;

class AKONADI_SEARCH_XAPIAN_EXPORT XapianDatabase
{
public:
    /**
     * Create the Xapian db at path \p path. The parameter \p
     * writeOnly locks the database as long as this object is
     * valid
     */
    XapianDatabase(const QString &path, bool writeOnly = false);
    ~XapianDatabase();

    void replaceDocument(uint id, const Xapian::Document &doc);
    void replaceDocument(uint id, const XapianDocument &doc);
    void deleteDocument(uint id);

    /**
     * Commit all the pending changes. This may not commit
     * at this instance as the db might be locked by another process
     * It emits the committed signal on completion
     */
    void commit();

    XapianDocument document(uint id);

    /**
     * A pointer to the actual db. Only use this when doing queries
     */
    Xapian::Database *db()
    {
        if (m_db) {
            m_db->reopen();
            return m_db;
        }
        return &m_wDb;
    }

    /**
     * Returns true if the XapianDatabase has changes which need to
     * be committed
     */
    bool haveChanges() const;

private:
    Xapian::Database *m_db = nullptr;
    Xapian::WritableDatabase m_wDb;

    typedef QPair<Xapian::docid, Xapian::Document> DocIdPair;
    QVector<DocIdPair> m_docsToAdd;
    QVector<uint> m_docsToRemove;

    std::string m_path;
    bool m_writeOnly = false;

    Xapian::WritableDatabase createWritableDb();
};

}
}

#endif // AKONADI_SEARCH_XAPIANDATABASE_H
