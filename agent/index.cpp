/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include <xapian.h>

#include "akonadi_indexer_agent_debug.h"
#include "akonotesindexer.h"
#include "calendarindexer.h"
#include "contactindexer.h"
#include "emailindexer.h"
#include "index.h"
#include "indexeditems.h"

#include <AkonadiCore/ServerManager>
#include <QDir>
#include <QStandardPaths>

using namespace Akonadi::Search::PIM;
Index::Index(QObject *parent)
    : QObject(parent)
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
        const auto dirs = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
        for (const QFileInfo &info : dirs) {
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
    for (const QString &mimeType : mimeTypes) {
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
    // We always get items of the same type
    AbstractIndexer *indexer = indexerForItem(items.first());
    if (!indexer) {
        return;
    }
    for (const Akonadi::Item &item : items) {
        try {
            indexer->move(item.id(), from.id(), to.id());
        } catch (const Xapian::Error &e) {
            qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Xapian error in indexer" << indexer << ":" << e.get_msg().c_str();
        }
    }
}

void Index::updateFlags(const Akonadi::Item::List &items, const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removedFlags)
{
    // We always get items of the same type
    AbstractIndexer *indexer = indexerForItem(items.first());
    if (!indexer) {
        return;
    }
    for (const Akonadi::Item &item : items) {
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
    for (Akonadi::Item::Id id : ids) {
        for (AbstractIndexer *indexer : indexers) {
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
    for (const Akonadi::Item &item : items) {
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
    // Remove items
    const auto indexers = indexersForMimetypes(col.contentMimeTypes());
    for (AbstractIndexer *indexer : indexers) {
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

void Index::move(const Akonadi::Collection &collection, const Akonadi::Collection &from, const Akonadi::Collection &to)
{
    if (m_collectionIndexer) {
        m_collectionIndexer->move(collection, from, to);
        m_collectionIndexer->commit();
    }
}

void Index::addIndexer(AbstractIndexer *indexer)
{
    m_listIndexer.append(indexer);
    const QStringList indexerMimetypes = indexer->mimeTypes();
    for (const QString &mimeType : indexerMimetypes) {
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
    for (AbstractIndexer *indexer : std::as_const(m_listIndexer)) {
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
