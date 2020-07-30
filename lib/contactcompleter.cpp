/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include <xapian.h>

#include "contactcompleter.h"
#include "query.h"
#include "akonadi_search_pim_debug.h"

#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QElapsedTimer>

using namespace Akonadi::Search::PIM;

ContactCompleter::ContactCompleter(const QString &prefix, int limit)
    : m_prefix(prefix.toLower())
    , m_limit(limit)
{
}

static QStringList processEnquire(Xapian::Enquire &enq, int limit)
{
    QElapsedTimer timer;
    timer.start();

    // Retrieves no results but provides statistics - it's very quick
    auto statsmset = enq.get_mset(0, 0);
    qCDebug(AKONADI_SEARCH_PIM_LOG) << "Query:" << QString::fromStdString(enq.get_query().get_description());
    qCDebug(AKONADI_SEARCH_PIM_LOG) << "Estimated matches:" << statsmset.get_matches_estimated();
    const int matchEstimate = statsmset.get_matches_estimated();

    QStringList list;
    list.reserve(std::min(limit, matchEstimate));
    int duplicates = 0;
    int firstItem = 0;
    // We run the query multiple times, since we may discard some results as duplicates.
    while (list.size() < limit) {
        // Always query the "limit"-count of results:
        //  * if estimate is less than limit, we make sure we don't miss results any due to wrong estimate
        //  * if estimate is more than limit, we don't want to query more documents than needed
        Xapian::MSet mset = enq.get_mset(firstItem, limit);
        if (mset.empty()) { // there are no more non-duplicate results
            break;
        }

        for (auto it = mset.begin(), end = mset.end(); it != end && list.size() < limit; ++it) {
            const auto entry = QString::fromStdString(it.get_document().get_data());
            // TODO: Be smarter about the deduplication by fixing the indexing code:
            // If we store mailbox name and address as separate named terms then we could deduplicate
            // purely based on the email address.
            if (!list.contains(entry, Qt::CaseInsensitive)) {
                qCDebug(AKONADI_SEARCH_PIM_LOG, "Match: \"%s\" (%d%%), docid %u", qUtf8Printable(entry), it.get_percent(), *it);
                list.push_back(entry);
            } else {
                ++duplicates;
                qCDebug(AKONADI_SEARCH_PIM_LOG, "Skipped duplicate match \"%s\" (%d%%) docid %u", qUtf8Printable(entry), it.get_percent(), *it);
            }
            ++firstItem;
        }
    }

    qCDebug(AKONADI_SEARCH_PIM_LOG) << "Collected" << list.size() << "results in" << timer.elapsed() << "ms, skipped" << duplicates << "duplicates.";
    return list;
}

QStringList ContactCompleter::complete()
{
    const QString dir = Query::defaultLocation(QStringLiteral("emailContacts"));
    Xapian::Database db;
    try {
        db = Xapian::Database(QFile::encodeName(dir).constData());
    } catch (const Xapian::DatabaseOpeningError &) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Xapian Database does not exist at " << dir;
        return QStringList();
    } catch (const Xapian::DatabaseCorruptError &) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Xapian Database corrupted";
        return QStringList();
    } catch (const Xapian::DatabaseError &e) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << QString::fromStdString(e.get_type()) << QString::fromStdString(e.get_description());
        return QStringList();
    } catch (...) {
        qCWarning(AKONADI_SEARCH_PIM_LOG) << "Random exception, but we do not want to crash";
        return QStringList();
    }

    Xapian::QueryParser parser;
    parser.set_database(db);

    const int flags = Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL;
    const Xapian::Query q = parser.parse_query(m_prefix.toStdString(), flags);

    Xapian::Enquire enq(db);
    enq.set_query(q);
    enq.set_sort_by_relevance();
    // TODO: extend the indexer to use value slots for the normalized email address so that
    // duplicates can be collapsed by Xapian::Enquire::set_collapse_key()

    int retryCount = 0;
    Q_FOREVER {
        try {
            return processEnquire(enq, m_limit);
        } catch (const Xapian::DatabaseCorruptError &e) {
            qCWarning(AKONADI_SEARCH_PIM_LOG) << "The emailContacts Xapian database is corrupted:" << QString::fromStdString(e.get_description());
            return QStringList();
        } catch (const Xapian::DatabaseModifiedError &e) {
            db.reopen();
            retryCount++;
            if (retryCount > 3) {
                qCWarning(AKONADI_SEARCH_PIM_LOG) << "The emailContacts Xapian database seems broken:" << QString::fromStdString(e.get_description());
                return QStringList();
            }
            continue; // try again
        }
    }
}
