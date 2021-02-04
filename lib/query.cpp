/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "query.h"
#include "akonadi_search_pim_debug.h"
#include "contactquery.h"

#include <QDebug>
#include <QVariant>

#include <AkonadiCore/ServerManager>
#include <QDir>
#include <QJsonDocument>
#include <QStandardPaths>

using namespace Akonadi::Search::PIM;

Query::Query()
{
}

Query::~Query()
{
}

Query *Query::fromJSON(const QByteArray &json)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if (doc.isNull()) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Could not parse json query" << error.errorString();
        return nullptr;
    }

    const QVariantMap result = doc.toVariant().toMap();
    const QString type = result[QStringLiteral("type")].toString().toLower();
    if (type != QLatin1String("contact")) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Can only handle contact queries";
        return nullptr;
    }

    auto cq = new ContactQuery();
    cq->matchName(result[QStringLiteral("name")].toString());
    cq->matchNickname(result[QStringLiteral("nick")].toString());
    cq->matchEmail(result[QStringLiteral("email")].toString());
    cq->matchUID(result[QStringLiteral("uid")].toString());
    cq->match(result[QStringLiteral("$")].toString());

    const QString criteria = result[QStringLiteral("matchCriteria")].toString().toLower();
    if (criteria == QLatin1String("exact")) {
        cq->setMatchCriteria(ContactQuery::ExactMatch);
    } else if (criteria == QLatin1String("startswith")) {
        cq->setMatchCriteria(ContactQuery::StartsWithMatch);
    }

    cq->setLimit(result[QStringLiteral("limit")].toInt());

    return cq;
}

QString Query::defaultLocation(const QString &dbName)
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
