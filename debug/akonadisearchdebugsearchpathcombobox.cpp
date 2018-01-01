/*
  Copyright (c) 2014-2018 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "akonadisearchdebugsearchpathcombobox.h"
#include <QStandardPaths>
#include <QDir>
#include <AkonadiCore/ServerManager>

using namespace Akonadi::Search;
AkonadiSearchDebugSearchPathComboBox::AkonadiSearchDebugSearchPathComboBox(QWidget *parent)
    : QComboBox(parent)
{
    initialize();
}

AkonadiSearchDebugSearchPathComboBox::~AkonadiSearchDebugSearchPathComboBox()
{

}

QString AkonadiSearchDebugSearchPathComboBox::searchPath()
{
    const int currentPathIndex = currentIndex();
    if (currentPathIndex > -1) {
        const QString value = pathFromEnum(static_cast<Akonadi::Search::AkonadiSearchDebugSearchPathComboBox::SearchType>(itemData(currentPathIndex).toInt()));
        return value;
    } else {
        return QString();
    }
}

void AkonadiSearchDebugSearchPathComboBox::initialize()
{
    addItem(QStringLiteral("Contacts"), Contacts);
    addItem(QStringLiteral("ContactCompleter"), ContactCompleter);
    addItem(QStringLiteral("Email"), Emails);
    addItem(QStringLiteral("Notes"), Notes);
    addItem(QStringLiteral("Calendars"), Calendars);
}

QString AkonadiSearchDebugSearchPathComboBox::pathFromEnum(SearchType type)
{
    switch (type) {
    case Contacts:
        return defaultLocations(QStringLiteral("contacts"));
    case ContactCompleter:
        return defaultLocations(QStringLiteral("emailContacts"));
    case Emails:
        return defaultLocations(QStringLiteral("email"));
    case Notes:
        return defaultLocations(QStringLiteral("notes"));
    case Calendars:
        return defaultLocations(QStringLiteral("calendars"));
    }
    return QString();
}

void AkonadiSearchDebugSearchPathComboBox::setSearchType(AkonadiSearchDebugSearchPathComboBox::SearchType type)
{
    const int indexType = findData(type);
    if (indexType >= 0) {
        setCurrentIndex(indexType);
    }
}

QString AkonadiSearchDebugSearchPathComboBox::defaultLocations(const QString &dbName)
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
