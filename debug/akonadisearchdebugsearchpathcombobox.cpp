/*
  SPDX-FileCopyrightText: 2014-2025 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akonadisearchdebugsearchpathcombobox.h"
using namespace Qt::Literals::StringLiterals;

#include <Akonadi/ServerManager>
#include <QDir>
#include <QStandardPaths>

using namespace Akonadi::Search;
AkonadiSearchDebugSearchPathComboBox::AkonadiSearchDebugSearchPathComboBox(QWidget *parent)
    : QComboBox(parent)
{
    initialize();
}

AkonadiSearchDebugSearchPathComboBox::~AkonadiSearchDebugSearchPathComboBox() = default;

QString AkonadiSearchDebugSearchPathComboBox::searchPath() const
{
    const int currentPathIndex = currentIndex();
    if (currentPathIndex > -1) {
        const QString value = pathFromEnum(static_cast<Akonadi::Search::AkonadiSearchDebugSearchPathComboBox::SearchType>(itemData(currentPathIndex).toInt()));
        return value;
    } else {
        return {};
    }
}

void AkonadiSearchDebugSearchPathComboBox::initialize()
{
    addItem(u"Contacts"_s, Contacts);
    addItem(u"ContactCompleter"_s, ContactCompleter);
    addItem(u"Email"_s, Emails);
    addItem(u"Notes"_s, Notes);
    addItem(u"Calendars"_s, Calendars);
}

QString AkonadiSearchDebugSearchPathComboBox::pathFromEnum(SearchType type) const
{
    switch (type) {
    case Contacts:
        return defaultLocations(u"contacts"_s);
    case ContactCompleter:
        return defaultLocations(u"emailContacts"_s);
    case Emails:
        return defaultLocations(u"email"_s);
    case Notes:
        return defaultLocations(u"notes"_s);
    case Calendars:
        return defaultLocations(u"calendars"_s);
    }
    return {};
}

void AkonadiSearchDebugSearchPathComboBox::setSearchType(AkonadiSearchDebugSearchPathComboBox::SearchType type)
{
    const int indexType = findData(type);
    if (indexType >= 0) {
        setCurrentIndex(indexType);
    }
}

const QString AkonadiSearchDebugSearchPathComboBox::defaultLocations(const QString &dbName) const
{
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
    return dbPath;
}

#include "moc_akonadisearchdebugsearchpathcombobox.cpp"
