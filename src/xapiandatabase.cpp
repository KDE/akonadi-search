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

#include <xapian.h>

#include "xapiandatabase.h"
#include "xapiandocument.h"
#include "akonadisearch_debug.h"

#include <QDir>

#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif

#include <chrono>
#include <thread>

using namespace Akonadi::Search;

namespace Akonadi {
namespace Search {

class XapianDatabasePrivate
{
public:
    ~XapianDatabasePrivate()
    {
        delete db;
    }

    Xapian::Database *db = nullptr;

    std::string path;
    bool readOnly = true;
};

}
}

XapianDatabase::XapianDatabase(const QString &path, bool readOnly)
    : d(new XapianDatabasePrivate)
{
    d->readOnly = readOnly;

    QDir().mkpath(path);
    d->path = path.toUtf8().constData();

    if (readOnly) {
        try {
            d->db = new Xapian::Database(d->path);
        } catch (const Xapian::DatabaseError &err) {
            qCWarning(AKONADISEARCH_LOG) << "Xapian database error:" << err.get_error_string();
            qCWarning(AKONADISEARCH_LOG) << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
        }
    } else {
        // FIXME: Don't block here, do this asynchronously
        for (int i = 1; i <= 20; ++i) {
            try {
                d->db = new Xapian::WritableDatabase(d->path, Xapian::DB_CREATE_OR_OPEN);
                break;
            } catch (const Xapian::DatabaseLockError &) {
                std::this_thread::sleep_for(std::chrono::milliseconds(i * 50));
            } catch (const Xapian::DatabaseModifiedError &) {
                std::this_thread::sleep_for(std::chrono::milliseconds(i * 50));
            } catch (const Xapian::DatabaseCreateError &err) {
                qCWarning(AKONADISEARCH_LOG) << "Xapian database error:" << err.get_error_string();
                qCWarning(AKONADISEARCH_LOG) << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
                delete d->db;
                d->db = nullptr;
            } catch (const Xapian::DatabaseCorruptError &err) {
                qCWarning(AKONADISEARCH_LOG) << "Xapian database corruption:" << err.get_error_string();
                qCWarning(AKONADISEARCH_LOG) << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
                delete d->db;
                d->db = nullptr;
            } catch (const Xapian::DatabaseError &err) {
                qCWarning(AKONADISEARCH_LOG) << "Xapian database error:" << err.get_error_string();
                qCWarning(AKONADISEARCH_LOG) << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
                delete d->db;
                d->db = nullptr;
            }
        }
    }
}

XapianDatabase::~XapianDatabase()
{
    delete d;
}

bool XapianDatabase::replaceDocument(uint id, const XapianDocument &doc)
{
    return replaceDocument(id, doc.xapianDocument());
}

bool XapianDatabase::replaceDocument(uint id, const Xapian::Document &doc)
{
    if (d->readOnly) {
        qCWarning(AKONADISEARCH_LOG) << "Database opened in read-only mode!";
        return false;
    }

    try {
        static_cast<Xapian::WritableDatabase*>(d->db)->replace_document(id, doc);
    } catch (const Xapian::DatabaseCorruptError &err) {
        qCWarning(AKONADISEARCH_LOG) << "Failed to write document due to Xapian db corruption:" << err.get_error_string();
        qCWarning(AKONADISEARCH_LOG) << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
        return false;
    } catch (const Xapian::DatabaseError &err) {
        qCWarning(AKONADISEARCH_LOG) << "Failed to write document due to Xapian db error:" << err.get_error_string();
        qCWarning(AKONADISEARCH_LOG) << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
        return false;
    }

    return true;
}

bool XapianDatabase::deleteDocument(uint id)
{
    if (id == 0) {
        return false;
    }

    if (d->readOnly) {
        qCWarning(AKONADISEARCH_LOG) << "Database opened in read-only mode!";
        return false;
    }

    try {
        static_cast<Xapian::WritableDatabase*>(d->db)->delete_document(id);
    } catch (const Xapian::DatabaseCorruptError &err) {
        qCWarning(AKONADISEARCH_LOG) << "Failed to delete document due to Xapian db corruption:" << err.get_error_string();
        qCWarning(AKONADISEARCH_LOG) << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
        return false;
    } catch (const Xapian::DatabaseError &err) {
        qCWarning(AKONADISEARCH_LOG) << "Failed to delete document due to Xapian db error:" << err.get_error_string();
        qCWarning(AKONADISEARCH_LOG) << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
        return false;
    }

    return true;
}

Xapian::Database *XapianDatabase::db()
{
    if (d->db) {
        d->db->reopen();
    }
    return d->db;
}

bool XapianDatabase::commit()
{
    if (d->readOnly) {
        qCWarning(AKONADISEARCH_LOG) << "Database opened in read-only mode!";
        return false;
    }

    try {
        static_cast<Xapian::WritableDatabase*>(d->db)->commit();
    } catch (const Xapian::DatabaseCorruptError &err) {
        qCWarning(AKONADISEARCH_LOG) << "Failed to commit trx due to Xapian db corruption:" << err.get_error_string();
        qCWarning(AKONADISEARCH_LOG) << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
        return false;
    } catch (const Xapian::DatabaseError &err) {
        qCWarning(AKONADISEARCH_LOG) << "Failed to commit trx due to Xapian db error:" << err.get_error_string();
        qCWarning(AKONADISEARCH_LOG) << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
        return false;
    }

#if defined(HAVE_MALLOC_TRIM)
    malloc_trim(0);
#endif

    return true;
}

XapianDocument XapianDatabase::document(uint id)
{
    try {
        return XapianDocument(d->db->get_document(id));
    } catch (const Xapian::DatabaseModifiedError &) {
        d->db->reopen();
        return document(id);
    } catch (const Xapian::DocNotFoundError &err) {
        qCDebug(AKONADISEARCH_LOG) << "No such document with ID" << id;
        return XapianDocument();
    } catch (const Xapian::Error &err) {
        qCWarning(AKONADISEARCH_LOG) << "Failed to get document from Xapian db:" << err.get_error_string();
        qCWarning(AKONADISEARCH_LOG) << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
        return XapianDocument();
    }

    return XapianDocument();
}
