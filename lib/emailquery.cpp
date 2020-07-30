/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include <xapian.h>

#include "emailquery.h"
#include "resultiterator_p.h"
#include "../search/email/agepostingsource.h"
#include "akonadi_search_pim_debug.h"

#include <QStandardPaths>
#include <QRegularExpression>
#include <QFile>

using namespace Akonadi::Search::PIM;

class Q_DECL_HIDDEN EmailQuery::Private
{
public:
    Private();

    QStringList involves;
    QStringList to;
    QStringList cc;
    QStringList bcc;
    QString from;

    QList<Akonadi::Collection::Id> collections;

    char important;
    char read;
    char attachment;

    QString matchString;
    QString subjectMatchString;
    QString bodyMatchString;

    EmailQuery::OpType opType = EmailQuery::OpAnd;
    int limit = 0;
    bool splitSearchMatchString = true;
};

EmailQuery::Private::Private()
    : important('0')
    , read('0')
    , attachment('0')
{
}

EmailQuery::EmailQuery()
    : Query()
    , d(new Private)
{
}

EmailQuery::~EmailQuery()
{
    delete d;
}

void EmailQuery::setSplitSearchMatchString(bool split)
{
    d->splitSearchMatchString = split;
}

void EmailQuery::setSearchType(EmailQuery::OpType op)
{
    d->opType = op;
}

void EmailQuery::addInvolves(const QString &email)
{
    d->involves << email;
}

void EmailQuery::setInvolves(const QStringList &involves)
{
    d->involves = involves;
}

void EmailQuery::addBcc(const QString &bcc)
{
    d->bcc << bcc;
}

void EmailQuery::setBcc(const QStringList &bcc)
{
    d->bcc = bcc;
}

void EmailQuery::setCc(const QStringList &cc)
{
    d->cc = cc;
}

void EmailQuery::setFrom(const QString &from)
{
    d->from = from;
}

void EmailQuery::addTo(const QString &to)
{
    d->to << to;
}

void EmailQuery::setTo(const QStringList &to)
{
    d->to = to;
}

void EmailQuery::addCc(const QString &cc)
{
    d->cc << cc;
}

void EmailQuery::addFrom(const QString &from)
{
    d->from = from;
}

void EmailQuery::addCollection(Akonadi::Collection::Id id)
{
    d->collections << id;
}

void EmailQuery::setCollection(const QList<Akonadi::Collection::Id> &collections)
{
    d->collections = collections;
}

int EmailQuery::limit() const
{
    return d->limit;
}

void EmailQuery::setLimit(int limit)
{
    d->limit = limit;
}

void EmailQuery::matches(const QString &match)
{
    d->matchString = match;
}

void EmailQuery::subjectMatches(const QString &subjectMatch)
{
    d->subjectMatchString = subjectMatch;
}

void EmailQuery::bodyMatches(const QString &bodyMatch)
{
    d->bodyMatchString = bodyMatch;
}

void EmailQuery::setAttachment(bool hasAttachment)
{
    d->attachment = hasAttachment ? 'T' : 'F';
}

void EmailQuery::setImportant(bool important)
{
    d->important = important ? 'T' : 'F';
}

void EmailQuery::setRead(bool read)
{
    d->read = read ? 'T' : 'F';
}

ResultIterator EmailQuery::exec()
{
    const QString dir = defaultLocation(QStringLiteral("email"));
    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(dir).constData());
    } catch (const Xapian::DatabaseOpeningError &) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Xapian Database does not exist at " << dir;
        return ResultIterator();
    } catch (const Xapian::DatabaseCorruptError &) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Xapian Database corrupted";
        return ResultIterator();
    } catch (const Xapian::DatabaseError &e) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Failed to open Xapian database:" << QString::fromStdString(e.get_description());
        return ResultIterator();
    } catch (...) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Random exception, but we do not want to crash";
        return ResultIterator();
    }

    QList<Xapian::Query> m_queries;

    if (!d->involves.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "F");
        parser.add_prefix("", "T");
        parser.add_prefix("", "CC");
        parser.add_prefix("", "BCC");

        // vHanda: Do we really need the query parser over here?
        for (const QString &str : qAsConst(d->involves)) {
            const QByteArray ba = str.toUtf8();
            m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    if (!d->from.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "F");
        const QByteArray ba = d->from.toUtf8();
        m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
    }

    if (!d->to.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "T");

        for (const QString &str : qAsConst(d->to)) {
            const QByteArray ba = str.toUtf8();
            m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    if (!d->cc.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "CC");

        for (const QString &str : qAsConst(d->cc)) {
            const QByteArray ba = str.toUtf8();
            m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    if (!d->bcc.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "BC");

        for (const QString &str : qAsConst(d->bcc)) {
            const QByteArray ba = str.toUtf8();
            m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
        }
    }

    if (!d->subjectMatchString.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "SU");
        parser.set_default_op(Xapian::Query::OP_AND);
        const QByteArray ba = d->subjectMatchString.toUtf8();
        m_queries << parser.parse_query(ba.constData(),
                                        Xapian::QueryParser::FLAG_PARTIAL);
    }

    if (!d->collections.isEmpty()) {
        Xapian::Query query;
        for (Akonadi::Collection::Id id : qAsConst(d->collections)) {
            QString c = QString::number(id);
            Xapian::Query q = Xapian::Query('C' + c.toStdString());

            query = Xapian::Query(Xapian::Query::OP_OR, query, q);
        }

        m_queries << query;
    }

    if (!d->bodyMatchString.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "BO");
        parser.set_default_op(Xapian::Query::OP_AND);
        const QByteArray ba = d->bodyMatchString.toUtf8();
        m_queries << parser.parse_query(ba.constData(), Xapian::QueryParser::FLAG_PARTIAL);
    }

    if (d->important == 'T') {
        m_queries << Xapian::Query("BI");
    } else if (d->important == 'F') {
        m_queries << Xapian::Query("BNI");
    }

    if (d->read == 'T') {
        m_queries << Xapian::Query("BR");
    } else if (d->read == 'F') {
        m_queries << Xapian::Query("BNR");
    }

    if (d->attachment == 'T') {
        m_queries << Xapian::Query("BA");
    } else if (d->attachment == 'F') {
        m_queries << Xapian::Query("BNA");
    }

    if (!d->matchString.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.set_default_op(Xapian::Query::OP_AND);
        if (d->splitSearchMatchString) {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            const QStringList list = d->matchString.split(QRegularExpression(QStringLiteral("\\s")), QString::SkipEmptyParts);
#else
            const QStringList list = d->matchString.split(QRegularExpression(QStringLiteral("\\s")), Qt::SkipEmptyParts);
#endif
            for (const QString &s : list) {
                const QByteArray ba = s.toUtf8();
                m_queries << parser.parse_query(ba.constData(),
                                                Xapian::QueryParser::FLAG_PARTIAL);
            }
        } else {
            const QByteArray ba = d->matchString.toUtf8();
            m_queries << parser.parse_query(ba.constData(),
                                            Xapian::QueryParser::FLAG_PARTIAL);
        }
    }
    Xapian::Query query;
    switch (d->opType) {
    case OpAnd:
        query = Xapian::Query(Xapian::Query::OP_AND, m_queries.begin(), m_queries.end());
        break;
    case OpOr:
        query = Xapian::Query(Xapian::Query::OP_OR, m_queries.begin(), m_queries.end());
        break;
    }

    AgePostingSource ps(0);
    query = Xapian::Query(Xapian::Query::OP_AND_MAYBE, query, Xapian::Query(&ps));

    try {
        Xapian::Enquire enquire(db);
        enquire.set_query(query);

        if (d->limit == 0) {
            d->limit = 1000000;
        }

        Xapian::MSet mset = enquire.get_mset(0, d->limit);

        ResultIterator iter;
        iter.d->init(mset);
        return iter;
    } catch (const Xapian::Error &e) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << QString::fromStdString(e.get_type()) << QString::fromStdString(e.get_description());
        return ResultIterator();
    }
}
