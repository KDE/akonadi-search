/*
 * Copyright (C) 2017  Daniel Vrátil <dvratil@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <xapian.h>

#include "store.h"
#include "akonadisearch_debug.h"
#include "registrar_p.h"
#include "resultiterator.h"
#include "resultiterator_p.h"
#include "xapiandatabase.h"
#include "xapiandocument.h"

#include "email/emailstore.h"
#include "contact/contactstore.h"
#include "incidence/incidencestore.h"
#include "note/notestore.h"
#include "collection/collectionstore.h"

#include <AkonadiCore/ServerManager>

#include <QStandardPaths>
#include <QDir>
#include <QTimer>

using namespace Akonadi::Search;

namespace {

Q_GLOBAL_STATIC(Registrar<Store>, sStores)

static const unsigned int MaxQueryLimit = 10^6;

}

namespace Akonadi {
namespace Search {

class StorePrivate
{
public:
    StorePrivate(Store *q);
    ~StorePrivate();

    bool ensureDb();
    QString dbPath(const QString &name) const;

    void newChange();

    QString dbName;
    XapianDatabase *db = nullptr;
    Store::OpenMode openMode = Store::ReadOnly;
    int changeCount = 0;
    int commitChangeCount = 0;
    QTimer *commitTimer;

private:
    Store * const q;
};

}
}

StorePrivate::StorePrivate(Store *q)
    : q(q)
{
    commitTimer = new QTimer;
    commitTimer->setSingleShot(true);
    QObject::connect(commitTimer, &QTimer::timeout,
                     [=]() { q->commit(); });
}

StorePrivate::~StorePrivate()
{
    delete commitTimer;
    if (db) {
        if (openMode == Store::WriteOnly) {
            db->commit();
        }
        delete db;
    }
}

bool StorePrivate::ensureDb()
{
    if (!db) {
        db = new XapianDatabase(dbPath(dbName), openMode == Store::ReadOnly);
    }

    return db && db->db();
}

void StorePrivate::newChange()
{
    ++changeCount;
    if (changeCount >= commitChangeCount) {
        q->commit();
    } else {
        commitTimer->start();
    }
}


QString StorePrivate::dbPath(const QString &dbName) const
{
    // First look into the old location from Baloo times in ~/.local/share/baloo,
    // because we don't migrate the database files automatically.
    QString basePath;
    bool hasInstanceIdentifier = Akonadi::ServerManager::hasInstanceIdentifier();
    if (hasInstanceIdentifier) {
        basePath = QStringLiteral("baloo/instances/%1").arg(Akonadi::ServerManager::instanceIdentifier());
    } else {
        basePath = QStringLiteral("baloo");
    }
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/%1/%2/").arg(basePath, dbName);
    if (QDir(dbPath).exists()) {
        return dbPath;
    }

    // If the database does not exist in old Baloo folders, than use the new
    // location in Akonadi's datadir in ~/.local/share/akonadi/search_db.
    if (hasInstanceIdentifier) {
        basePath = QStringLiteral("akonadi/instance/%1/search_db").arg(Akonadi::ServerManager::instanceIdentifier());
    } else {
        basePath = QStringLiteral("akonadi/search_db");
    }
    dbPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/%1/%2/").arg(basePath, dbName);
    QDir().mkpath(dbPath);
    return dbPath;
}



Store *Store::create(const QString &mimeType)
{
    if (!sStores.exists()) {
        sStores->registerForType<EmailStore>();
        sStores->registerForType<ContactStore>();
        sStores->registerForType<IncidenceStore>();
        sStores->registerForType<NoteStore>();
        sStores->registerForType<CollectionStore>();
    }

    return sStores->instantiate(mimeType);
}

Store::Store()
    : d(new StorePrivate(this))
{
}

Store::~Store()
{
    delete d;
}

void Store::setAutoCommit(int changeCount, int timeoutMs)
{
    d->commitChangeCount = changeCount;
    if (timeoutMs > 0) {
        d->commitTimer->setInterval(timeoutMs);
        d->commitTimer->start();
    } else {
        d->commitTimer->stop();
    }
}


QString Store::dbName() const
{
    return d->dbName;
}

void Store::setDbName(const QString &name)
{
    d->dbName = name;
}

Store::OpenMode Store::openMode() const
{
    return d->openMode;
}

void Store::setOpenMode(OpenMode openMode)
{
    d->openMode = openMode;
}

bool Store::index(qint64 id, const QByteArray &serializedIndex)
{
    if (!d->ensureDb()) {
        return false;
    }

    // FIXME: Xapian allows up to 2^32 documents, while Akonadi can deal with IDs
    // up to 2^64. However it's very unlikely that someone will ever have a
    // problem with running out of 32bit Item Ids....
    const auto doc = Xapian::Document::unserialise({ serializedIndex.constData(),
                                                     static_cast<std::string::size_type>(serializedIndex.size()) });

    d->newChange();

    return d->db->replaceDocument(static_cast<uint>(id), doc);
}

bool Store::removeItem(qint64 id)
{
    if (!d->ensureDb()) {
        return false;
    }

    d->newChange();

    return d->db->deleteDocument(static_cast<uint>(id));
}

bool Store::removeCollection(qint64 id)
{
    if (!d->ensureDb()) {
        return false;
    }

    try {
        Xapian::Database *db = d->db->db();
        Xapian::Query query(XapianDocument::collectionId(id).constData());
        Xapian::Enquire enquire(*db);
        enquire.set_query(query);

        Xapian::MSet mset = enquire.get_mset(0, db->get_doccount());
        Xapian::MSetIterator end(mset.end());
        for (Xapian::MSetIterator it = mset.begin(); it != end; ++it) {
            removeItem(*it);
        }

        d->newChange();

        return true;
    } catch (const Xapian::DocNotFoundError &) {
        return true;
    } catch (const Xapian::DatabaseError &err) {
        qCWarning(AKONADISEARCH_LOG) << "Error when removing from DB:" << err.get_error_string();
        return false;
    }
}

bool Store::move(const qint64 id, qint64 srcCollection, qint64 destCollection)
{
    if (!d->ensureDb()) {
        return false;
    }

    XapianDocument doc(d->db->document(id));
    doc.removeTerm(XapianDocument::collectionId(srcCollection));
    doc.addCollectionTerm(destCollection);

    d->newChange();

    return d->db->replaceDocument(id, doc);
}

bool Store::copy(qint64 id, qint64 srcCollection, qint64 destId, qint64 destCollection)
{
    if (!d->ensureDb()) {
        return false;
    }

    XapianDocument doc(d->db->document(id));
    doc.removeTerm(XapianDocument::collectionId(srcCollection));
    doc.addCollectionTerm(destCollection);

    d->newChange();

    return d->db->replaceDocument(destId, doc);
}

bool Store::commit()
{
    if (!d->ensureDb()) {
        return false;
    }

    d->changeCount = 0;
    if (d->commitTimer->isActive()) {
        d->commitTimer->stop();
    }
    return d->db->commit();
}

ResultIterator Store::search(const QByteArray &serializedQuery, unsigned int limit)
{
    if (!d->ensureDb()) {
        return ResultIterator();
    }

    Xapian::Enquire enq(*d->db->db());
    const auto query = Xapian::Query::unserialise({ serializedQuery.constData(),
                                                    static_cast<std::string::size_type>(serializedQuery.size()) });
    enq.set_query(query);

    Xapian::MSet mset;
    try {
        mset = enq.get_mset(0, limit > 0 ? limit : MaxQueryLimit);
    } catch (const Xapian::InvalidArgumentError &err) {
        qCWarning(AKONADISEARCH_LOG) << "Xapian query error:" << err.get_error_string();
        qCWarning(AKONADISEARCH_LOG) << err.get_msg().c_str() << err.get_context().c_str() << err.get_description().c_str();
        return {};
    }

    ResultIterator iter;
    iter.d->init(mset);
    return iter;
}
