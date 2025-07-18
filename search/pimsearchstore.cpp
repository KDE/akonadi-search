/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */
#include "pimsearchstore.h"
using namespace Qt::Literals::StringLiterals;

#include "query.h"
#include "term.h"

#include <Akonadi/ServerManager>
#include <QDir>
#include <QStandardPaths>
#include <QUrl>
#include <QUrlQuery>

using namespace Akonadi::Search;

PIMSearchStore::PIMSearchStore(QObject *parent)
    : XapianSearchStore(parent)
{
}

QStringList PIMSearchStore::types()
{
    return QStringList() << u"Akonadi"_s;
}

QString PIMSearchStore::findDatabase(const QString &dbName) const
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

Xapian::Query PIMSearchStore::constructQuery(const QString &property, const QVariant &value, Term::Comparator com)
{
    if (value.isNull()) {
        return {};
    }

    QString prop = property.toLower();
    if (m_boolProperties.contains(prop)) {
        QString p = m_prefix.value(prop);
        if (p.isEmpty()) {
            return {};
        }

        std::string term("B");
        bool isTrue = false;

        if (value.isNull()) {
            isTrue = true;
        }
        if (value.userType() == QMetaType::Bool) {
            isTrue = value.toBool();
        }

        if (isTrue) {
            term += p.toStdString();
        } else {
            term += 'N' + p.toStdString();
        }

        return Xapian::Query(term);
    }

    if (m_boolWithValue.contains(prop)) {
        std::string term(m_prefix.value(prop).toStdString());
        std::string val(value.toString().toStdString());
        return Xapian::Query(term + val);
    }

    if (m_valueProperties.contains(prop)
        && (com == Term::Equal || com == Term::Greater || com == Term::GreaterEqual || com == Term::Less || com == Term::LessEqual)) {
        qlonglong numVal = value.toLongLong();
        if (com == Term::Greater) {
            ++numVal;
        }
        if (com == Term::Less) {
            --numVal;
        }
        int valueNumber = m_valueProperties.value(prop);
        if (com == Term::GreaterEqual || com == Term::Greater) {
            return Xapian::Query(Xapian::Query::OP_VALUE_GE, valueNumber, QString::number(numVal).toStdString());
        } else if (com == Term::LessEqual || com == Term::Less) {
            return Xapian::Query(Xapian::Query::OP_VALUE_LE, valueNumber, QString::number(numVal).toStdString());
        } else if (com == Term::Equal) {
            const Xapian::Query gtQuery(Xapian::Query::OP_VALUE_GE, valueNumber, QString::number(numVal).toStdString());
            const Xapian::Query ltQuery(Xapian::Query::OP_VALUE_LE, valueNumber, QString::number(numVal).toStdString());
            return Xapian::Query(Xapian::Query::OP_AND, gtQuery, ltQuery);
        }
    } else if ((com == Term::Contains || com == Term::Equal) && m_prefix.contains(prop)) {
        Xapian::QueryParser parser;
        parser.set_database(*xapianDb());

        std::string p = m_prefix.value(prop).toStdString();
        std::string str(value.toString().toStdString());
        int flags = Xapian::QueryParser::FLAG_DEFAULT;
        if (com == Term::Contains) {
            flags |= Xapian::QueryParser::FLAG_PARTIAL;
        }
        return parser.parse_query(str, flags, p);
    }
    return Xapian::Query(value.toString().toStdString());
}

QUrl PIMSearchStore::constructUrl(const Xapian::docid &docid)
{
    QUrl url;
    url.setScheme(u"akonadi"_s);

    QUrlQuery query;
    query.addQueryItem(u"item"_s, QString::number(docid));
    url.setQuery(query);

    return url;
}

#include "moc_pimsearchstore.cpp"
