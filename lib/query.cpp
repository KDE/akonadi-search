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

#include <QVariant>

#include <Akonadi/ServerManager>
#include <QDir>
#include <QJsonDocument>
#include <QStandardPaths>

using namespace Qt::Literals::StringLiterals;
using namespace Akonadi::Search::PIM;

Query::Query() = default;

Query::~Query() = default;

Query *Query::fromJSON(const QByteArray &json)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if (doc.isNull()) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Could not parse json query" << error.errorString();
        return nullptr;
    }

    const QVariantMap result = doc.toVariant().toMap();
    const QString type = result[u"type"_s].toString().toLower();
    if (type != "contact"_L1) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Can only handle contact queries";
        return nullptr;
    }

    auto cq = new ContactQuery();
    cq->matchName(result[u"name"_s].toString());
    cq->matchNickname(result[u"nick"_s].toString());
    cq->matchEmail(result[u"email"_s].toString());
    cq->matchUID(result[u"uid"_s].toString());
    cq->match(result[u"$"_s].toString());

    const QString criteria = result[u"matchCriteria"_s].toString().toLower();
    if (criteria == "exact"_L1) {
        cq->setMatchCriteria(ContactQuery::ExactMatch);
    } else if (criteria == "startswith"_L1) {
        cq->setMatchCriteria(ContactQuery::StartsWithMatch);
    }

    cq->setLimit(result[u"limit"_s].toInt());

    return cq;
}

QString Query::defaultLocation(const QString &dbName)
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
