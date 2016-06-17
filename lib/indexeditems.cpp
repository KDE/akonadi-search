/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2016 Laurent Montel <montel@kde.org>
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
#include "indexeditems.h"
#include "akonadi_search_pim_debug.h"
#include <QStandardPaths>
#include <QHash>
#include <QDir>
#include <AkonadiCore/ServerManager>
#include <xapian/error.h>
#include <xapian/database.h>
#include <xapian/query.h>
#include <xapian/enquire.h>


using namespace Akonadi::Search::PIM;

class Akonadi::Search::PIM::IndexedItemsPrivate
{
public:
    IndexedItemsPrivate()
    {

    }
    QString dbPath(const QString &dbName) const;
    QString emailIndexingPath() const;
    QString collectionIndexingPath() const;
    QString calendarIndexingPath() const;
    QString akonotesIndexingPath() const;
    QString emailContactsIndexingPath() const;
    QString contactIndexingPath() const;

    mutable QHash<QString, QString> m_cachePath;
    QString m_overridePrefixPath;
    qlonglong indexedItems(const qlonglong id);
    qlonglong indexedItemsInDatabase(const std::string &term, const QString &dbPath) const;
};


QString IndexedItemsPrivate::dbPath(const QString &dbName) const
{
    const QString cachedPath = m_cachePath.value(dbName);
    if (!cachedPath.isEmpty()) {
        return cachedPath;
    }
    if (!m_overridePrefixPath.isEmpty()) {
        const QString path = QString::fromLatin1("%1/%2/").arg(m_overridePrefixPath, dbName);
        m_cachePath.insert(dbName, path);
        return path;
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
        m_cachePath.insert(dbName, dbPath);
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
    const std::string term = QString::fromLatin1("C%1").arg(id).toStdString();
    return indexedItemsInDatabase(term, emailIndexingPath())
           + indexedItemsInDatabase(term, contactIndexingPath())
           + indexedItemsInDatabase(term, akonotesIndexingPath())
           + indexedItemsInDatabase(term, calendarIndexingPath());
}

IndexedItems::IndexedItems(QObject *parent)
    : QObject(parent),
      d(new Akonadi::Search::PIM::IndexedItemsPrivate())
{

}

IndexedItems::~IndexedItems()
{
    delete d;
}

void IndexedItems::setOverrideDbPrefixPath(const QString &path)
{
    d->m_overridePrefixPath = path;
}

qlonglong IndexedItems::indexedItems(const qlonglong id)
{
    return d->indexedItems(id);
}
