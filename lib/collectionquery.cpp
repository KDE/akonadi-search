/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include <xapian.h>

#include "akonadi_search_pim_debug.h"
#include "collectionquery.h"
#include "resultiterator_p.h"
#include <QFile>
#include <QList>

#include <QStandardPaths>

using namespace Akonadi::Search::PIM;

struct CollectionQuery::Private {
    QStringList ns;
    QStringList mimeType;
    QString nameString;
    QString identifierString;
    QString pathString;
    QString databaseDir;
    int limit;
};

CollectionQuery::CollectionQuery()
    : Query()
    , d(new Private)
{
    d->databaseDir = defaultLocation(QStringLiteral("collections"));
    d->limit = 0;
}

CollectionQuery::~CollectionQuery() = default;

void CollectionQuery::setDatabaseDir(const QString &dir)
{
    d->databaseDir = dir;
}

void CollectionQuery::nameMatches(const QString &match)
{
    d->nameString = match;
}

void CollectionQuery::identifierMatches(const QString &match)
{
    d->identifierString = match;
}

void CollectionQuery::pathMatches(const QString &match)
{
    d->pathString = match;
}

void CollectionQuery::setNamespace(const QStringList &ns)
{
    d->ns = ns;
}

void CollectionQuery::setMimetype(const QStringList &mt)
{
    d->mimeType = mt;
}

void CollectionQuery::setLimit(int limit)
{
    d->limit = limit;
}

ResultIterator CollectionQuery::exec()
{
    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(d->databaseDir).toStdString());
    } catch (const Xapian::DatabaseError &e) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Failed to open Xapian database:" << d->databaseDir << "; error:" << QString::fromStdString(e.get_error_string());
        return ResultIterator();
    }

    QList<Xapian::Query> queries;

    if (!d->nameString.isEmpty()) {
        // qDebug() << "searching by name";
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "N");
        parser.set_default_op(Xapian::Query::OP_AND);
        queries << parser.parse_query(d->nameString.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
    }

    if (!d->identifierString.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "I");
        parser.set_default_op(Xapian::Query::OP_AND);
        queries << parser.parse_query(d->identifierString.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);
    }

    if (!d->pathString.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "P");
        parser.set_default_op(Xapian::Query::OP_AND);
        queries << parser.parse_query(d->pathString.toStdString(), Xapian::QueryParser::FLAG_PARTIAL | Xapian::QueryParser::FLAG_PHRASE);
    }

    if (!d->ns.isEmpty()) {
        QList<Xapian::Query> queryList;
        queryList.reserve(d->ns.count());
        for (const QString &n : std::as_const(d->ns)) {
            const QByteArray term = "NS" + n.toUtf8();
            queryList << Xapian::Query(term.constData());
        }
        queries << Xapian::Query(Xapian::Query::OP_OR, queryList.begin(), queryList.end());
    }

    if (!d->mimeType.isEmpty()) {
        QList<Xapian::Query> queryList;
        queryList.reserve(d->mimeType.count());
        for (const QString &n : std::as_const(d->mimeType)) {
            const QByteArray term = "M" + n.toUtf8();
            queryList << Xapian::Query(term.constData());
        }
        queries << Xapian::Query(Xapian::Query::OP_OR, queryList.begin(), queryList.end());
    }

    Xapian::Query query = Xapian::Query(Xapian::Query::OP_AND, queries.begin(), queries.end());

    if (d->limit == 0) {
        d->limit = 1000000;
    }

    Xapian::Enquire enquire(db);
    enquire.set_query(query);

    Xapian::MSet mset = enquire.get_mset(0, d->limit);

    ResultIterator iter;
    iter.d->init(mset);
    return iter;
}
