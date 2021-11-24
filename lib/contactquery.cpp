/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include <xapian.h>

#include "akonadi_search_pim_debug.h"
#include "contactquery.h"
#include "resultiterator_p.h"

#include <QDebug>
#include <QFile>
#include <QList>
#include <QStandardPaths>

using namespace Akonadi::Search::PIM;

class Akonadi::Search::PIM::ContactQueryPrivate
{
public:
    QString name;
    QString nick;
    QString email;
    QString uid;
    QString any;

    int limit;
    ContactQuery::MatchCriteria criteria;
};

ContactQuery::ContactQuery()
    : Query()
    , d(new ContactQueryPrivate)
{
    d->criteria = StartsWithMatch;
}

ContactQuery::~ContactQuery() = default;

void ContactQuery::matchName(const QString &name)
{
    d->name = name;
}

void ContactQuery::matchNickname(const QString &nick)
{
    d->nick = nick;
}

void ContactQuery::matchEmail(const QString &email)
{
    d->email = email;
}

void ContactQuery::matchUID(const QString &uid)
{
    d->uid = uid;
}

void ContactQuery::match(const QString &str)
{
    d->any = str;
}

int ContactQuery::limit() const
{
    return d->limit;
}

void ContactQuery::setLimit(int limit)
{
    d->limit = limit;
}

ContactQuery::MatchCriteria ContactQuery::matchCriteria() const
{
    return d->criteria;
}

void ContactQuery::setMatchCriteria(ContactQuery::MatchCriteria m)
{
    d->criteria = m;
}

ResultIterator ContactQuery::exec()
{
    const QString dir = defaultLocation(QStringLiteral("contacts"));
    Xapian::Database db;

    try {
        db = Xapian::Database(QFile::encodeName(dir).toStdString());
    } catch (const Xapian::DatabaseOpeningError &) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Xapian Database does not exist at " << dir;
        return {};
    } catch (const Xapian::DatabaseCorruptError &) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Xapian Database corrupted";
        return {};
    } catch (const Xapian::DatabaseError &e) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Failed to open Xapian database:" << QString::fromStdString(e.get_error_string());
        return {};
    } catch (...) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Random exception, but we do not want to crash";
        return {};
    }

    QList<Xapian::Query> m_queries;

    if (d->criteria == ExactMatch) {
        if (!d->any.isEmpty()) {
            const QByteArray ba = d->any.toUtf8();
            m_queries << Xapian::Query(ba.constData());
        }

        if (!d->name.isEmpty()) {
            const QByteArray ba = "NA" + d->name.toUtf8();
            m_queries << Xapian::Query(ba.constData());
        }

        if (!d->nick.isEmpty()) {
            const QByteArray ba = "NI" + d->nick.toUtf8();
            m_queries << Xapian::Query(ba.constData());
        }

        if (!d->email.isEmpty()) {
            const QByteArray ba = d->email.toUtf8();
            m_queries << Xapian::Query(ba.constData());
        }

        if (!d->uid.isEmpty()) {
            const QByteArray ba = "UID" + d->uid.toUtf8();
            m_queries << Xapian::Query(ba.constData());
        }
    } else if (d->criteria == StartsWithMatch) {
        if (!d->any.isEmpty()) {
            Xapian::QueryParser parser;
            parser.set_database(db);
            const QByteArray ba = d->any.toUtf8();
            m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
        }

        if (!d->name.isEmpty()) {
            Xapian::QueryParser parser;
            parser.set_database(db);
            parser.add_prefix("", "NA");
            const QByteArray ba = d->name.toUtf8();
            m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
        }

        if (!d->nick.isEmpty()) {
            Xapian::QueryParser parser;
            parser.set_database(db);
            parser.add_prefix("", "NI");
            const QByteArray ba = d->nick.toUtf8();
            m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
        }

        // FIXME: Check for exact match?
        if (!d->email.isEmpty()) {
            Xapian::QueryParser parser;
            parser.set_database(db);
            const QByteArray ba = d->email.toUtf8();
            m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
        }

        if (!d->uid.isEmpty()) {
            Xapian::QueryParser parser;
            parser.set_database(db);
            parser.add_prefix("", "UID");
            const QByteArray ba = d->uid.toUtf8();
            m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }
    try {
        Xapian::Query query(Xapian::Query::OP_OR, m_queries.begin(), m_queries.end());
        qCDebug(AKONADI_SEARCH_PIM_LOG) << query.get_description().c_str();

        Xapian::Enquire enquire(db);
        enquire.set_query(query);

        if (d->limit == 0) {
            d->limit = 10000;
        }

        Xapian::MSet matches = enquire.get_mset(0, d->limit);

        ResultIterator iter;
        iter.d->init(matches);
        return iter;
    } catch (const Xapian::Error &e) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << QString::fromStdString(e.get_type()) << QString::fromStdString(e.get_description());
        return {};
    }
}
