/*
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#ifndef AKONADI_SEARCH_XAPIANDATABASE_H
#define AKONADI_SEARCH_XAPIANDATABASE_H

#include "search_xapian_export.h"
#include <xapian.h>

#include <QPair>
#include <QString>
#include <QVector>

namespace Akonadi
{
namespace Search
{
class XapianDocument;

/** Xapian database. */
class AKONADI_SEARCH_XAPIAN_EXPORT XapianDatabase
{
public:
    /**
     * Create the Xapian db at path \p path. The parameter \p
     * writeOnly locks the database as long as this object is
     * valid
     */
    explicit XapianDatabase(const QString &path, bool writeOnly = false);
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
    const bool m_writeOnly = false;

    Xapian::WritableDatabase createWritableDb();
};
}
}

#endif // AKONADI_SEARCH_XAPIANDATABASE_H
