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

#include "indexeditems.h"
#include <AkonadiCore/ServerManager>
#include <QDir>
#include <QStandardPaths>
#include <xapian.h>

using namespace Akonadi::Search::PIM;
Index::Index(QObject *parent)
    : QObject(parent),
      m_collectionIndexer(nullptr)
{
    m_indexedItems = new IndexedItems(this);
    m_commitTimer.setInterval(1000);
    m_commitTimer.setSingleShot(true);
    connect(&m_commitTimer, &QTimer::timeout, this, &Index::commit);
}

Index::~Index()
{
    delete m_collectionIndexer;
    m_collectionIndexer = nullptr;
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
    m_collectionIndexer = nullptr;
    qDeleteAll(m_listIndexer);
    m_listIndexer.clear();
    m_indexer.clear();

    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Removing database";
    removeDir(m_indexedItems->emailIndexingPath());
    removeDir(m_indexedItems->contactIndexingPath());
    removeDir(m_indexedItems->emailContactsIndexingPath());
    removeDir(m_indexedItems->akonotesIndexingPath());
    removeDir(m_indexedItems->calendarIndexingPath());
    removeDir(m_indexedItems->collectionIndexingPath());
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
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << " No indexer found for item";
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
    Q_FOREACH (Akonadi::Item::Id id, ids) {
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
    AbstractIndexer *indexer = nullptr;
    try {
        QDir().mkpath(m_indexedItems->emailIndexingPath());
        QDir().mkpath(m_indexedItems->emailContactsIndexingPath());
        indexer = new EmailIndexer(m_indexedItems->emailIndexingPath(), m_indexedItems->emailContactsIndexingPath());
        addIndexer(indexer);
    } catch (const Xapian::DatabaseError &e) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Failed to create email indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Random exception, but we do not want to crash";
    }

    try {
        QDir().mkpath(m_indexedItems->contactIndexingPath());
        indexer = new ContactIndexer(m_indexedItems->contactIndexingPath());
        addIndexer(indexer);
    } catch (const Xapian::DatabaseError &e) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Failed to create contact indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Random exception, but we do not want to crash";
    }

    try {
        QDir().mkpath(m_indexedItems->akonotesIndexingPath());
        indexer = new AkonotesIndexer(m_indexedItems->akonotesIndexingPath());
        addIndexer(indexer);
    } catch (const Xapian::DatabaseError &e) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Failed to create akonotes indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Random exception, but we do not want to crash";
    }

    try {
        QDir().mkpath(m_indexedItems->calendarIndexingPath());
        indexer = new CalendarIndexer(m_indexedItems->calendarIndexingPath());
        addIndexer(indexer);
    } catch (const Xapian::DatabaseError &e) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Failed to create akonotes indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete indexer;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Random exception, but we do not want to crash";
    }

    try {
        QDir().mkpath(m_indexedItems->collectionIndexingPath());
        m_collectionIndexer = new CollectionIndexer(m_indexedItems->collectionIndexingPath());
    } catch (const Xapian::DatabaseError &e) {
        delete m_collectionIndexer;
        m_collectionIndexer = nullptr;
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Failed to create akonotes indexer:" << QString::fromStdString(e.get_msg());
    } catch (...) {
        delete m_collectionIndexer;
        m_collectionIndexer = nullptr;
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

void Index::findIndexed(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId)
{
    m_indexedItems->findIndexed(indexed, collectionId);
}

qlonglong Index::indexedItems(const qlonglong id)
{
    return m_indexedItems->indexedItems(id);
}

void Index::setOverrideDbPrefixPath(const QString &path)
{
    m_indexedItems->setOverrideDbPrefixPath(path);
}
