/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2016-2025 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include <xapian.h>

#include "akonadi_search_pim_debug.h"
#include "indexeditems.h"

#include <Akonadi/ServerManager>
#include <QDir>
#include <QHash>
#include <QStandardPaths>

using namespace Akonadi::Search::PIM;
using namespace Qt::Literals::StringLiterals;

class Akonadi::Search::PIM::IndexedItemsPrivate
{
public:
    IndexedItemsPrivate() = default;

    [[nodiscard]] QString dbPath(const QString &dbName) const;
    [[nodiscard]] QString emailIndexingPath() const;
    [[nodiscard]] QString collectionIndexingPath() const;
    [[nodiscard]] QString calendarIndexingPath() const;
    [[nodiscard]] QString akonotesIndexingPath() const;
    [[nodiscard]] QString emailContactsIndexingPath() const;
    [[nodiscard]] QString contactIndexingPath() const;

    mutable QHash<QString, QString> m_cachePath;
    QString m_overridePrefixPath;
    [[nodiscard]] qlonglong indexedItems(const qlonglong id);
    [[nodiscard]] qlonglong indexedItemsInDatabase(const std::string &term, const QString &dbPath) const;
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
        const QString path = u"%1/%2/"_s.arg(m_overridePrefixPath, dbName);
        m_cachePath.insert(dbName, path);
        return path;
    }

    // First look into the old location from Baloo times in ~/.local/share/baloo,
    // because we don't migrate the database files automatically.
    QString basePath;
    bool hasInstanceIdentifier = Akonadi::ServerManager::hasInstanceIdentifier();
    if (hasInstanceIdentifier) {
        basePath = u"baloo/instances/%1"_s.arg(Akonadi::ServerManager::instanceIdentifier());
    } else {
        basePath = u"baloo"_s;
    }
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + u"/%1/%2/"_s.arg(basePath, dbName);
    if (QDir(dbPath).exists()) {
        m_cachePath.insert(dbName, dbPath);
        return dbPath;
    }

    // If the database does not exist in old Baloo folders, than use the new
    // location in Akonadi's datadir in ~/.local/share/akonadi/search_db.
    if (hasInstanceIdentifier) {
        basePath = u"akonadi/instance/%1/search_db"_s.arg(Akonadi::ServerManager::instanceIdentifier());
    } else {
        basePath = u"akonadi/search_db"_s;
    }
    dbPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + u"/%1/%2/"_s.arg(basePath, dbName);
    QDir().mkpath(dbPath);
    m_cachePath.insert(dbName, dbPath);
    return dbPath;
}

QString IndexedItemsPrivate::emailIndexingPath() const
{
    return dbPath(u"email"_s);
}

QString IndexedItemsPrivate::contactIndexingPath() const
{
    return dbPath(u"contacts"_s);
}

QString IndexedItemsPrivate::emailContactsIndexingPath() const
{
    return dbPath(u"emailContacts"_s);
}

QString IndexedItemsPrivate::akonotesIndexingPath() const
{
    return dbPath(u"notes"_s);
}

QString IndexedItemsPrivate::calendarIndexingPath() const
{
    return dbPath(u"calendars"_s);
}

QString IndexedItemsPrivate::collectionIndexingPath() const
{
    return dbPath(u"collections"_s);
}

qlonglong IndexedItemsPrivate::indexedItemsInDatabase(const std::string &term, const QString &dbPath) const
{
    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(dbPath).toStdString());
    } catch (const Xapian::DatabaseError &e) {
        qCCritical(AKONADI_SEARCH_PIM_LOG) << "Failed to open database" << dbPath << ":" << QString::fromStdString(e.get_msg());
        return 0;
    }
    return db.get_termfreq(term);
}

qlonglong IndexedItemsPrivate::indexedItems(const qlonglong id)
{
    const std::string term = u"C%1"_s.arg(id).toStdString();
    return indexedItemsInDatabase(term, emailIndexingPath()) + indexedItemsInDatabase(term, contactIndexingPath())
        + indexedItemsInDatabase(term, akonotesIndexingPath()) + indexedItemsInDatabase(term, calendarIndexingPath());
}

void IndexedItemsPrivate::findIndexedInDatabase(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId, const QString &dbPath)
{
    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(dbPath).toStdString());
    } catch (const Xapian::DatabaseError &e) {
        qCCritical(AKONADI_SEARCH_PIM_LOG) << "Failed to open database" << dbPath << ":" << QString::fromStdString(e.get_msg());
        return;
    }
    const std::string term = u"C%1"_s.arg(collectionId).toStdString();
    const Xapian::Query query(term);
    Xapian::Enquire enquire(db);
    enquire.set_query(query);

    auto getResults = [&enquire, &indexed]() {
        Xapian::MSet mset;
        mset = enquire.get_mset(0, UINT_MAX);
        Xapian::MSetIterator it = mset.begin();
        for (auto result : mset) {
            indexed << result;
        }
    };

    try {
        getResults();
    } catch (const Xapian::DatabaseModifiedError &e) {
        qCCritical(AKONADI_SEARCH_PIM_LOG) << "Failed to read database" << dbPath << ":" << QString::fromStdString(e.get_msg());
        qCCritical(AKONADI_SEARCH_PIM_LOG) << "Calling reopen() on database" << dbPath << "and trying again";
        if (db.reopen()) { // only try again once
            try {
                getResults();
            } catch (const Xapian::DatabaseModifiedError &e) {
                qCCritical(AKONADI_SEARCH_PIM_LOG) << "Failed to query database" << dbPath << "even after calling reopen()";
            }
        }
    } catch (const Xapian::DatabaseCorruptError &e) {
        qCCritical(AKONADI_SEARCH_PIM_LOG) << "Failed to query database" << dbPath << ":" << QString::fromStdString(e.get_msg());
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

IndexedItems::~IndexedItems() = default;

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

#include "moc_indexeditems.cpp"
