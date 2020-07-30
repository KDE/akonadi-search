/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2016-2020 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include <xapian.h>

#include "indexeditems.h"
#include "akonadi_search_pim_debug.h"

#include <QStandardPaths>
#include <QHash>
#include <QDir>
#include <AkonadiCore/ServerManager>

using namespace Akonadi::Search::PIM;

class Akonadi::Search::PIM::IndexedItemsPrivate
{
public:
    IndexedItemsPrivate()
    {
    }

    Q_REQUIRED_RESULT QString dbPath(const QString &dbName) const;
    Q_REQUIRED_RESULT QString emailIndexingPath() const;
    Q_REQUIRED_RESULT QString collectionIndexingPath() const;
    Q_REQUIRED_RESULT QString calendarIndexingPath() const;
    Q_REQUIRED_RESULT QString akonotesIndexingPath() const;
    Q_REQUIRED_RESULT QString emailContactsIndexingPath() const;
    Q_REQUIRED_RESULT QString contactIndexingPath() const;

    mutable QHash<QString, QString> m_cachePath;
    QString m_overridePrefixPath;
    Q_REQUIRED_RESULT qlonglong indexedItems(const qlonglong id);
    Q_REQUIRED_RESULT qlonglong indexedItemsInDatabase(const std::string &term, const QString &dbPath) const;
    void findIndexedInDatabase(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId, const QString &dbPath);
    void findIndexed(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId);
};

QString IndexedItemsPrivate::dbPath(const QString &dbName) const
{
    const QString cachedPath = m_cachePath.value(dbName);
    if (!cachedPath.isEmpty()) {
        return cachedPath;
    }
    if (!m_overridePrefixPath.isEmpty()) {
        const QString path = QStringLiteral("%1/%2/").arg(m_overridePrefixPath, dbName);
        m_cachePath.insert(dbName, path);
        return path;
    }

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
        m_cachePath.insert(dbName, dbPath);
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
    m_cachePath.insert(dbName, dbPath);
    return dbPath;
}

QString IndexedItemsPrivate::emailIndexingPath() const
{
    return dbPath(QStringLiteral("email"));
}

QString IndexedItemsPrivate::contactIndexingPath() const
{
    return dbPath(QStringLiteral("contacts"));
}

QString IndexedItemsPrivate::emailContactsIndexingPath() const
{
    return dbPath(QStringLiteral("emailContacts"));
}

QString IndexedItemsPrivate::akonotesIndexingPath() const
{
    return dbPath(QStringLiteral("notes"));
}

QString IndexedItemsPrivate::calendarIndexingPath() const
{
    return dbPath(QStringLiteral("calendars"));
}

QString IndexedItemsPrivate::collectionIndexingPath() const
{
    return dbPath(QStringLiteral("collections"));
}

qlonglong IndexedItemsPrivate::indexedItemsInDatabase(const std::string &term, const QString &dbPath) const
{
    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(dbPath).constData());
    } catch (const Xapian::DatabaseError &e) {
        qCCritical(AKONADI_SEARCH_PIM_LOG) << "Failed to open database" << dbPath << ":" << QString::fromStdString(e.get_msg());
        return 0;
    }
    return db.get_termfreq(term);
}

qlonglong IndexedItemsPrivate::indexedItems(const qlonglong id)
{
    const std::string term = QStringLiteral("C%1").arg(id).toStdString();
    return indexedItemsInDatabase(term, emailIndexingPath())
           + indexedItemsInDatabase(term, contactIndexingPath())
           + indexedItemsInDatabase(term, akonotesIndexingPath())
           + indexedItemsInDatabase(term, calendarIndexingPath());
}

void IndexedItemsPrivate::findIndexedInDatabase(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId, const QString &dbPath)
{
    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(dbPath).constData());
    } catch (const Xapian::DatabaseError &e) {
        qCCritical(AKONADI_SEARCH_PIM_LOG) << "Failed to open database" << dbPath << ":" << QString::fromStdString(e.get_msg());
        return;
    }
    const std::string term = QStringLiteral("C%1").arg(collectionId).toStdString();
    Xapian::Query query(term);
    Xapian::Enquire enquire(db);
    enquire.set_query(query);

    Xapian::MSet mset = enquire.get_mset(0, UINT_MAX);
    Xapian::MSetIterator it = mset.begin();
    for (; it != mset.end(); it++) {
        indexed << *it;
    }
}

void IndexedItemsPrivate::findIndexed(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId)
{
    findIndexedInDatabase(indexed, collectionId, emailIndexingPath());
    findIndexedInDatabase(indexed, collectionId, contactIndexingPath());
    findIndexedInDatabase(indexed, collectionId, akonotesIndexingPath());
    findIndexedInDatabase(indexed, collectionId, calendarIndexingPath());
}

IndexedItems::IndexedItems(QObject *parent)
    : QObject(parent)
    , d(new Akonadi::Search::PIM::IndexedItemsPrivate())
{
}

IndexedItems::~IndexedItems()
{
    delete d;
}

void IndexedItems::setOverrideDbPrefixPath(const QString &path)
{
    d->m_overridePrefixPath = path;
    d->m_cachePath.clear();
}

qlonglong IndexedItems::indexedItems(const qlonglong id)
{
    return d->indexedItems(id);
}

void IndexedItems::findIndexedInDatabase(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId, const QString &dbPath)
{
    d->findIndexedInDatabase(indexed, collectionId, dbPath);
}

void IndexedItems::findIndexed(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId)
{
    d->findIndexed(indexed, collectionId);
}

QString IndexedItems::emailIndexingPath() const
{
    return d->emailIndexingPath();
}

QString IndexedItems::collectionIndexingPath() const
{
    return d->collectionIndexingPath();
}

QString IndexedItems::calendarIndexingPath() const
{
    return d->calendarIndexingPath();
}

QString IndexedItems::akonotesIndexingPath() const
{
    return d->akonotesIndexingPath();
}

QString IndexedItems::emailContactsIndexingPath() const
{
    return d->emailContactsIndexingPath();
}

QString IndexedItems::contactIndexingPath() const
{
    return d->contactIndexingPath();
}
