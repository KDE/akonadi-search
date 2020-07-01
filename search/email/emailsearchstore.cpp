/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "emailsearchstore.h"
#include "term.h"
#include "query.h"
#include "agepostingsource.h"

using namespace Akonadi::Search;

EmailSearchStore::EmailSearchStore(QObject *parent)
    : PIMSearchStore(parent)
{
    m_prefix.insert(QStringLiteral("from"), QStringLiteral("F"));
    m_prefix.insert(QStringLiteral("to"), QStringLiteral("T"));
    m_prefix.insert(QStringLiteral("cc"), QStringLiteral("CC"));
    m_prefix.insert(QStringLiteral("bcc"), QStringLiteral("BC"));
    m_prefix.insert(QStringLiteral("subject"), QStringLiteral("SU"));
    m_prefix.insert(QStringLiteral("collection"), QStringLiteral("C"));
    m_prefix.insert(QStringLiteral("replyto"), QStringLiteral("RT"));
    m_prefix.insert(QStringLiteral("organization"), QStringLiteral("O"));
    m_prefix.insert(QStringLiteral("listid"), QStringLiteral("LI"));
    m_prefix.insert(QStringLiteral("resentfrom"), QStringLiteral("RF"));
    m_prefix.insert(QStringLiteral("xloop"), QStringLiteral("XL"));
    m_prefix.insert(QStringLiteral("xmailinglist"), QStringLiteral("XML"));
    m_prefix.insert(QStringLiteral("xspamflag"), QStringLiteral("XSF"));

    m_prefix.insert(QStringLiteral("body"), QStringLiteral("BO"));
    m_prefix.insert(QStringLiteral("headers"), QStringLiteral("HE"));

    // TODO: Add body flag?
    // TODO: Add tags?

    // Boolean Flags
    m_prefix.insert(QStringLiteral("isimportant"), QStringLiteral("I"));
    m_prefix.insert(QStringLiteral("istoact"), QStringLiteral("T"));
    m_prefix.insert(QStringLiteral("iswatched"), QStringLiteral("W"));
    m_prefix.insert(QStringLiteral("isdeleted"), QStringLiteral("D"));
    m_prefix.insert(QStringLiteral("isspam"), QStringLiteral("S"));
    m_prefix.insert(QStringLiteral("isreplied"), QStringLiteral("E"));
    m_prefix.insert(QStringLiteral("isignored"), QStringLiteral("G"));
    m_prefix.insert(QStringLiteral("isforwarded"), QStringLiteral("F"));
    m_prefix.insert(QStringLiteral("issent"), QStringLiteral("N"));
    m_prefix.insert(QStringLiteral("isqueued"), QStringLiteral("Q"));
    m_prefix.insert(QStringLiteral("isham"), QStringLiteral("H"));
    m_prefix.insert(QStringLiteral("isread"), QStringLiteral("R"));
    m_prefix.insert(QStringLiteral("hasattachment"), QStringLiteral("A"));
    m_prefix.insert(QStringLiteral("isencrypted"), QStringLiteral("C"));
    m_prefix.insert(QStringLiteral("hasinvitation"), QStringLiteral("V"));

    m_boolProperties << QStringLiteral("isimportant") << QStringLiteral("istoact") << QStringLiteral("iswatched") << QStringLiteral("isdeleted") << QStringLiteral("isspam")
                     << QStringLiteral("isreplied") << QStringLiteral("isignored") << QStringLiteral("isforwarded") << QStringLiteral("issent") << QStringLiteral("isqueued")
                     << QStringLiteral("isham") << QStringLiteral("isread") << QStringLiteral("hasattachment") << QStringLiteral("isencrypted") << QStringLiteral("hasinvitation");

    m_valueProperties.insert(QStringLiteral("date"), 0);
    m_valueProperties.insert(QStringLiteral("size"), 1);
    m_valueProperties.insert(QStringLiteral("onlydate"), 2);

    setDbPath(findDatabase(QStringLiteral("email")));
}

QStringList EmailSearchStore::types()
{
    return QStringList() << QStringLiteral("Akonadi") << QStringLiteral("Email");
}

Xapian::Query EmailSearchStore::constructQuery(const QString &property, const QVariant &value, Term::Comparator com)
{
    //TODO is this special case necessary? maybe we can also move it to PIM
    if (com == Term::Contains) {
        if (!m_prefix.contains(property.toLower())) {
            return Xapian::Query();
        }
    }
    return PIMSearchStore::constructQuery(property, value, com);
}

QString EmailSearchStore::text(int queryId)
{
    Xapian::Document doc = docForQuery(queryId);

    QMutexLocker lock(&m_mutex);
    std::string data;
    try {
        data = doc.get_data();
    } catch (const Xapian::Error &) {
        // Nothing to do, move along
    }

    QString subject = QString::fromUtf8(data.c_str(), data.length());
    if (subject.isEmpty()) {
        return QStringLiteral("No Subject");
    }

    return subject;
}

Xapian::Query EmailSearchStore::finalizeQuery(const Xapian::Query &query)
{
    return Xapian::Query(Xapian::Query::OP_AND_MAYBE, query, Xapian::Query(new AgePostingSource(0)));
}
