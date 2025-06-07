/*
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "xapiandatabase.h"
#include "xapiandocument.h"

#include "akonadi_search_xapian_debug.h"
#include <QDir>

#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif

#include <chrono>
#include <thread>

using namespace Akonadi::Search;

XapianDatabase::XapianDatabase(const QString &path, bool writeOnly)
    : m_writeOnly(writeOnly)
{
    QDir().mkpath(path);
    m_path = path.toStdString();

    if (!writeOnly) {
        try {
            createWritableDb();
            m_db = new Xapian::Database(m_path);
        } catch (const Xapian::DatabaseError &err) {
            qCWarning(AKONADI_SEARCH_XAPIAN_LOG) << "Serious Error:" << err.get_error_string();
            qCWarning(AKONADI_SEARCH_XAPIAN_LOG) << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
        }

        // Possible errors - DatabaseLock error
        // Corrupt and InvalidID error
    } else {
        m_wDb = createWritableDb();
    }
}

XapianDatabase::~XapianDatabase()
{
    delete m_db;
}

void XapianDatabase::replaceDocument(uint id, const XapianDocument &doc)
{
    replaceDocument(id, doc.doc());
}

void XapianDatabase::replaceDocument(uint id, const Xapian::Document &doc)
{
    if (m_writeOnly) {
        try {
            m_wDb.replace_document(id, doc);
        } catch (const Xapian::Error &) {
        }
        return;
    }
    m_docsToAdd << qMakePair(id, doc);
}

void XapianDatabase::deleteDocument(uint id)
{
    if (id == 0) {
        return;
    }

    if (m_writeOnly) {
        try {
            m_wDb.delete_document(id);
        } catch (const Xapian::Error &) {
        }
        return;
    }
    m_docsToRemove << id;
}

bool XapianDatabase::haveChanges() const
{
    return !m_docsToAdd.isEmpty() || !m_docsToRemove.isEmpty();
}

void XapianDatabase::commit()
{
    if (m_writeOnly) {
        try {
            m_wDb.commit();
        } catch (const Xapian::Error &err) {
            qCWarning(AKONADI_SEARCH_XAPIAN_LOG) << err.get_error_string();
        }
        return;
    }

    if (!haveChanges()) {
        return;
    }

    Xapian::WritableDatabase wdb = createWritableDb();

    qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << "Adding:" << m_docsToAdd.size() << "docs";
    for (const DocIdPair &doc : std::as_const(m_docsToAdd)) {
        try {
            wdb.replace_document(doc.first, doc.second);
        } catch (const Xapian::Error &) {
        }
    }

    qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << "Removing:" << m_docsToRemove.size() << "docs";
    for (Xapian::docid id : std::as_const(m_docsToRemove)) {
        try {
            wdb.delete_document(id);
        } catch (const Xapian::Error &) {
        }
    }

    try {
        wdb.commit();
        m_db->reopen();
    } catch (const Xapian::Error &err) {
        qCWarning(AKONADI_SEARCH_XAPIAN_LOG) << err.get_error_string();
    }
    qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << "Xapian Committed";

    m_docsToAdd.clear();
    m_docsToRemove.clear();

#if defined(HAVE_MALLOC_TRIM)
    malloc_trim(0);
#endif
}

XapianDocument XapianDatabase::document(uint id)
{
    try {
        Xapian::Document xdoc;
        if (m_writeOnly) {
            xdoc = m_wDb.get_document(id);
        } else {
            xdoc = m_db->get_document(id);
        }
        return XapianDocument(xdoc);
    } catch (const Xapian::DatabaseModifiedError &) {
        m_db->reopen();
        return document(id);
    } catch (const Xapian::Error &) {
        return {};
    }
}

Xapian::WritableDatabase XapianDatabase::createWritableDb()
{
    // We need to keep sleeping for a required amount, until we reach
    // a threshold. That's when we decide to abort?
    for (int i = 1; i <= 20; ++i) {
        try {
            Xapian::WritableDatabase wdb(m_path, Xapian::DB_CREATE_OR_OPEN);
            return wdb;
        } catch (const Xapian::DatabaseLockError &) {
            std::this_thread::sleep_for(std::chrono::milliseconds(i * 50));
        } catch (const Xapian::DatabaseModifiedError &) {
            std::this_thread::sleep_for(std::chrono::milliseconds(i * 50));
        } catch (const Xapian::DatabaseCreateError &err) {
            qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << err.get_error_string();
            return {};
        } catch (const Xapian::DatabaseCorruptError &err) {
            qCWarning(AKONADI_SEARCH_XAPIAN_LOG) << "Database Corrupted - What did you do?";
            qCWarning(AKONADI_SEARCH_XAPIAN_LOG) << err.get_error_string();
            return {};
        } catch (...) {
            qCWarning(AKONADI_SEARCH_XAPIAN_LOG) << "Bananana Error";
            return {};
        }
    }

    qCWarning(AKONADI_SEARCH_XAPIAN_LOG) << "Could not obtain lock for Xapian Database. This is bad";
    return {};
}
