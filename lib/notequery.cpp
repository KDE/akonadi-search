/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2020 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include <xapian.h>

#include "notequery.h"
#include "resultiterator_p.h"

#include <QList>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>

using namespace Akonadi::Search::PIM;

class Q_DECL_HIDDEN NoteQuery::Private
{
public:
    Private()
        : limit(0)
    {
    }

    QString title;
    QString note;
    int limit;
};

NoteQuery::NoteQuery()
    : Query()
    , d(new Private)
{
}

NoteQuery::~NoteQuery()
{
    delete d;
}

void NoteQuery::matchTitle(const QString &title)
{
    d->title = title;
}

void NoteQuery::matchNote(const QString &note)
{
    d->note = note;
}

void NoteQuery::setLimit(int limit)
{
    d->limit = limit;
}

int NoteQuery::limit() const
{
    return d->limit;
}

ResultIterator NoteQuery::exec()
{
    const QString dir = defaultLocation(QStringLiteral("notes"));

    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(dir).constData());
    } catch (const Xapian::DatabaseOpeningError &) {
        qWarning() << "Xapian Database does not exist at " << dir;
        return ResultIterator();
    } catch (const Xapian::DatabaseCorruptError &) {
        qWarning() << "Xapian Database corrupted";
        return ResultIterator();
    } catch (const Xapian::DatabaseError &e) {
        qWarning() << "Failed to open Xapian database:" << QString::fromStdString(e.get_error_string());
        return ResultIterator();
    } catch (...) {
        qWarning() << "Random exception, but we do not want to crash";
        return ResultIterator();
    }

    QList<Xapian::Query> m_queries;

    if (!d->note.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "BO");

        const QByteArray baNote = d->note.toUtf8();
        m_queries << parser.parse_query(baNote.constData(), Xapian::QueryParser::FLAG_PARTIAL);
    }

    if (!d->title.isEmpty()) {
        Xapian::QueryParser parser;
        parser.set_database(db);
        parser.add_prefix("", "SU");
        parser.set_default_op(Xapian::Query::OP_AND);

        const QByteArray baTitle = d->title.toUtf8();
        m_queries << parser.parse_query(baTitle.constData(), Xapian::QueryParser::FLAG_PARTIAL);
    }
    try {
        Xapian::Query query(Xapian::Query::OP_OR, m_queries.begin(), m_queries.end());
        //qDebug() << query.get_description().c_str();

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
        qWarning() << QString::fromStdString(e.get_type()) << QString::fromStdString(e.get_description());
        return ResultIterator();
    }
}
