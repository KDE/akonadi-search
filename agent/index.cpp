/*
 * Copyright 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "index.h"
#include "akonadi_indexer_agent_debug.h"
#include "emailindexer.h"
#include "contactindexer.h"
#include "akonotesindexer.h"
#include "calendarindexer.h"
#include <AkonadiCore/ServerManager>
#include <QDir>
#include <QStandardPaths>
#include <xapian/error.h>
#include <xapian/database.h>
#include <xapian/query.h>
#include <xapian/enquire.h>

Index::Index(QObject *parent)
    : QObject(parent),
      m_collectionIndexer(Q_NULLPTR)
{
    m_commitTimer.setInterval(1000);
    m_commitTimer.setSingleShot(true);
    connect(&m_commitTimer, &QTimer::timeout, this, &Index::commit);
}

Index::~Index()
{
    delete m_collectionIndexer;
    m_collectionIndexer = Q_NULLPTR;
    qDeleteAll(m_listIndexer);
}

static void removeDir(const QString &dirName)
{
    QDir dir(dirName);
    if (dir.exists(dirName)) {
        Q_FOREACH (const QFileInfo &info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                removeDir(info.absoluteFilePath());
            } else {
                QFile::remove(info.absoluteFilePath());
            }
        }
        dir.rmdir(dirName);
    }
}

void Index::removeDatabase()
{
    delete m_collectionIndexer;
    m_collectionIndexer = Q_NULLPTR;
    qDeleteAll(m_listIndexer);
    m_listIndexer.clear();
    m_indexer.clear();

    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Removing database";
    removeDir(emailIndexingPath());
    removeDir(contactIndexingPath());
    removeDir(emailContactsIndexingPath());
    removeDir(akonotesIndexingPath());
    removeDir(calendarIndexingPath());
    removeDir(collectionIndexingPath());
}

AbstractIndexer *Index::indexerForItem(const Akonadi::Item &item) const
{
    return m_indexer.value(item.mimeType());
}

QList<AbstractIndexer *> Index::indexersForMimetypes(const QStringList &mimeTypes) const
{
    QList<AbstractIndexer *> indexers;
    Q_FOREACH (const QString &mimeType, mimeTypes) {
        AbstractIndexer *i = m_indexer.value(mimeType);
        if (i) {
            indexers.append(i);
        }
    }
    return indexers;
}

bool Index::haveIndexerForMimeTypes(const QStringList &mimeTypes)
{
    return !indexersForMimetypes(mimeTypes).isEmpty();
}

void Index::index(const Akonadi::Item &item)
{
    AbstractIndexer *indexer = indexerForItem(item);
    if (!indexer) {
        return;
    }

    try {
        indexer->index(item);
    } catch (const Xapian::Error &e) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
    }
}

void Index::move(const Akonadi::Item::List &items, const Akonadi::Collection &from, const Akonadi::Collection &to)
{
    //We always get items of the same type
    AbstractIndexer *indexer = indexerForItem(items.first());
    if (!indexer) {
        return;
    }
    Q_FOREACH (const Akonadi::Item &item, items) {
        try {
            indexer->move(item.id(), from.id(), to.id());
        } catch (const Xapian::Error &e) {
            qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
}

void Index::updateFlags(const Akonadi::Item::List &items, const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removedFlags)
{
    //We always get items of the same type
    AbstractIndexer *indexer = indexerForItem(items.first());
    if (!indexer) {
        return;
    }
    Q_FOREACH (const Akonadi::Item &item, items) {
        try {
            indexer->updateFlags(item, addedFlags, removedFlags);
        } catch (const Xapian::Error &e) {
            qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
}

void Index::remove(const QSet<Akonadi::Item::Id> &ids, const QStringList &mimeTypes)
{
    const QList<AbstractIndexer *> indexers = indexersForMimetypes(mimeTypes);
    Q_FOREACH (const Akonadi::Item::Id &id, ids) {
        Q_FOREACH (AbstractIndexer *indexer, indexers) {
            try {
                indexer->remove(Akonadi::Item(id));
            } catch (const Xapian::Error &e) {
                qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
            }
        }
    }
}

void Index::remove(const Akonadi::Item::List &items)
{
    AbstractIndexer *indexer = indexerForItem(items.first());
    if (!indexer) {
        return;
    }
    Q_FOREACH (const Akonadi::Item &item, items) {
        try {
            indexer->remove(item);
        } catch (const Xapian::Error &e) {
            qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
}

void Index::index(const Akonadi::Collection &collection)
{
    if (m_collectionIndexer) {
        m_collectionIndexer->index(collection);
        m_collectionIndexer->commit();
    }
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "indexed " << collection.id();
}

void Index::change(const Akonadi::Collection &col)
{
    if (m_collectionIndexer) {
        m_collectionIndexer->change(col);
        m_collectionIndexer->commit();
    }
}

void Index::remove(const Akonadi::Collection &col)
{
    //Remove items
    Q_FOREACH (AbstractIndexer *indexer, indexersForMimetypes(col.contentMimeTypes())) {
        try {
            indexer->remove(col);
        } catch (const Xapian::Error &e) {
            qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }

    if (m_collectionIndexer) {
        m_collectionIndexer->remove(col);
        m_collectionIndexer->commit();
    }
}

void Index::move(const Akonadi::Collection &collection,
                 const Akonadi::Collection &from,
                 const Akonadi::Collection &to)
{
    if (m_collectionIndexer) {
        m_collectionIndexer->move(collection, from, to);
        m_collectionIndexer->commit();
    }
}

void Index::addIndexer(AbstractIndexer *indexer)
{
    m_listIndexer.append(indexer);
    Q_FOREACH (const QString &mimeType, indexer->mimeTypes()) {
        m_indexer.insert(mimeType, indexer);
    }
}

bool Index::createIndexers()
{
    AbstractIndexer *indexer = Q_NULLPTR;
    try {
        QDir().mkpath(emailIndexingPath());
        QDir().mkpath(emailContactsIndexingPath());
        indexer = new EmailIndexer(emailIndexingPath(), emailContactsIndexingPath());
        addIndexer(indexer);
    } catch (const Xapian::DatabaseError &e) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Failed to create email indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Random exception, but we do not want to crash";
    }

    try {
        QDir().mkpath(contactIndexingPath());
        indexer = new ContactIndexer(contactIndexingPath());
        addIndexer(indexer);
    } catch (const Xapian::DatabaseError &e) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Failed to create contact indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Random exception, but we do not want to crash";
    }

    try {
        QDir().mkpath(akonotesIndexingPath());
        indexer = new AkonotesIndexer(akonotesIndexingPath());
        addIndexer(indexer);
    } catch (const Xapian::DatabaseError &e) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Failed to create akonotes indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Random exception, but we do not want to crash";
    }

    try {
        QDir().mkpath(calendarIndexingPath());
        indexer = new CalendarIndexer(calendarIndexingPath());
        addIndexer(indexer);
    } catch (const Xapian::DatabaseError &e) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Failed to create akonotes indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Random exception, but we do not want to crash";
    }

    try {
        QDir().mkpath(collectionIndexingPath());
        m_collectionIndexer = new CollectionIndexer(collectionIndexingPath());
    } catch (const Xapian::DatabaseError &e) {
        delete m_collectionIndexer;
        m_collectionIndexer = Q_NULLPTR;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Failed to create akonotes indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete m_collectionIndexer;
        m_collectionIndexer = Q_NULLPTR;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Random exception, but we do not want to crash";
    }

    return !m_indexer.isEmpty();
}

void Index::scheduleCommit()
{
    if (!m_commitTimer.isActive()) {
        m_commitTimer.start();
    }
}

void Index::commit()
{
    m_commitTimer.stop();
    Q_FOREACH (AbstractIndexer *indexer, m_listIndexer) {
        try {
            indexer->commit();
        } catch (const Xapian::Error &e) {
            qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
}

void Index::findIndexedInDatabase(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId, const QString &dbPath)
{
    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(dbPath).constData());
    } catch (const Xapian::DatabaseError &e) {
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Failed to open database" << dbPath << ":" << QString::fromStdString(e.get_msg());
        return;
    }
    const std::string term = QString::fromLatin1("C%1").arg(collectionId).toStdString();
    Xapian::Query query(term);
    Xapian::Enquire enquire(db);
    enquire.set_query(query);

    Xapian::MSet mset = enquire.get_mset(0, UINT_MAX);
    Xapian::MSetIterator it = mset.begin();
    for (; it != mset.end(); it++) {
        indexed << *it;
    }
}

void Index::findIndexed(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId)
{
    findIndexedInDatabase(indexed, collectionId, emailIndexingPath());
    findIndexedInDatabase(indexed, collectionId, contactIndexingPath());
    findIndexedInDatabase(indexed, collectionId, akonotesIndexingPath());
    findIndexedInDatabase(indexed, collectionId, calendarIndexingPath());
}

qlonglong Index::indexedItems(const qlonglong id)
{
    const std::string term = QString::fromLatin1("C%1").arg(id).toStdString();
    return indexedItemsInDatabase(term, emailIndexingPath())
           + indexedItemsInDatabase(term, contactIndexingPath())
           + indexedItemsInDatabase(term, akonotesIndexingPath())
           + indexedItemsInDatabase(term, calendarIndexingPath());
}

qlonglong Index::indexedItemsInDatabase(const std::string &term, const QString &dbPath) const
{
    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(dbPath).constData());
    } catch (const Xapian::DatabaseError &e) {
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Failed to open database" << dbPath << ":" << QString::fromStdString(e.get_msg());
        return 0;
    }
    return db.get_termfreq(term);
}

void Index::setOverrideDbPrefixPath(const QString &path)
{
    m_overridePrefixPath = path;
}

QString Index::dbPath(const QString &dbName) const
{
    if (!m_overridePrefixPath.isEmpty()) {
        return QString::fromLatin1("%1/%2/").arg(m_overridePrefixPath, dbName);
    }

    // First look into the old location from Baloo times in ~/.local/share/baloo,
    // because we don't migrate the database files automatically.
    QString basePath;
    if (Akonadi::ServerManager::hasInstanceIdentifier()) {
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
    if (Akonadi::ServerManager::hasInstanceIdentifier()) {
        basePath = QStringLiteral("akonadi/instance/%1/search_db").arg(Akonadi::ServerManager::instanceIdentifier());
    } else {
        basePath = QStringLiteral("akonadi/search_db");
    }
    dbPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/%1/%2/").arg(basePath, dbName);
    QDir().mkpath(dbPath);
    return dbPath;
}

QString Index::emailIndexingPath() const
{
    return dbPath(QStringLiteral("email"));
}

QString Index::contactIndexingPath() const
{
    return dbPath(QStringLiteral("contacts"));
}

QString Index::emailContactsIndexingPath() const
{
    return dbPath(QStringLiteral("emailContacts"));
}

QString Index::akonotesIndexingPath() const
{
    return dbPath(QStringLiteral("notes"));
}

QString Index::calendarIndexingPath() const
{
    return dbPath(QStringLiteral("calendars"));
}

QString Index::collectionIndexingPath() const
{
    return dbPath(QStringLiteral("collections"));
}
