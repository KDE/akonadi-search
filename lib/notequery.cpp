/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2024 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include <xapian.h>

#include "akonadi_search_pim_debug.h"
#include "notequery.h"
#include "resultiterator_p.h"

#include <QDebug>
#include <QFile>
#include <QList>
#include <QStandardPaths>

using namespace Akonadi::Search::PIM;

class Akonadi::Search::PIM::NoteQueryPrivate
{
public:
    NoteQueryPrivate() = default;

    QString title;
    QString note;
    int limit = 0;
};

NoteQuery::NoteQuery()
    : Query()
    , d(new NoteQueryPrivate)
{
}

NoteQuery::~NoteQuery() = default;

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
        // qDebug() << query.get_description().c_str();

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
