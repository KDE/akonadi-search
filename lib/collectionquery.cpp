/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2014  Christian Mollekopf <mollekopf@kolabsys.com>
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
#include "collectionquery.h"
#include "resultiterator_p.h"
#include "xapian.h"

#include <QList>
#include <QFile>

#include <KDebug>
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
    : Query(),
      d(new Private)
{
    d->databaseDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("baloo/collections/");
    d->limit = 0;
}

CollectionQuery::~CollectionQuery()
{
    delete d;
}

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
        db = Xapian::Database(QFile::encodeName(d->databaseDir).constData());
    } catch (const Xapian::DatabaseError &e) {
        kWarning() << "Failed to open Xapian database:" << QString::fromStdString(e.get_error_string());
        return ResultIterator();
    }

    QList<Xapian::Query> queries;

    if (!d->nameString.isEmpty()) {
        kDebug() << "searching by name";
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "N");
        parser.set_default_op(Xapian::Query::OP_AND);
        queries << parser.parse_query(d->nameString.toUtf8().constData(),
                                      Xapian::QueryParser::FLAG_PARTIAL);
    }

    if (!d->identifierString.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "I");
        parser.set_default_op(Xapian::Query::OP_AND);
        queries << parser.parse_query(d->identifierString.toUtf8().constData(),
                                      Xapian::QueryParser::FLAG_PARTIAL);
    }

    if (!d->pathString.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "P");
        parser.set_default_op(Xapian::Query::OP_AND);
        queries << parser.parse_query(d->pathString.toUtf8().constData(),
                                      Xapian::QueryParser::FLAG_PARTIAL | Xapian::QueryParser::FLAG_PHRASE);
    }

    if (!d->ns.isEmpty()) {
        QList<Xapian::Query> queryList;
        Q_FOREACH (const QString &n, d->ns) {
            const QByteArray term = "NS" + n.toUtf8();
            queryList << Xapian::Query(term.constData());
        }
        queries << Xapian::Query(Xapian::Query::OP_OR, queryList.begin(), queryList.end());
    }

    if (!d->mimeType.isEmpty()) {
        QList<Xapian::Query> queryList;
        Q_FOREACH (const QString &n, d->mimeType) {
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

