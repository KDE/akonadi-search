/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "emailsearchstore.h"
using namespace Qt::Literals::StringLiterals;

#include "agepostingsource.h"
#include "query.h"
#include "term.h"

using namespace Akonadi::Search;

EmailSearchStore::EmailSearchStore(QObject *parent)
    : PIMSearchStore(parent)
{
    m_prefix.insert(u"from"_s, u"F"_s);
    m_prefix.insert(u"to"_s, u"T"_s);
    m_prefix.insert(u"cc"_s, u"CC"_s);
    m_prefix.insert(u"bcc"_s, u"BC"_s);
    m_prefix.insert(u"subject"_s, u"SU"_s);
    m_prefix.insert(u"collection"_s, u"C"_s);
    m_prefix.insert(u"replyto"_s, u"RT"_s);
    m_prefix.insert(u"organization"_s, u"O"_s);
    m_prefix.insert(u"listid"_s, u"LI"_s);
    m_prefix.insert(u"resentfrom"_s, u"RF"_s);
    m_prefix.insert(u"xloop"_s, u"XL"_s);
    m_prefix.insert(u"xmailinglist"_s, u"XML"_s);
    m_prefix.insert(u"xspamflag"_s, u"XSF"_s);

    m_prefix.insert(u"body"_s, u"BO"_s);
    m_prefix.insert(u"headers"_s, u"HE"_s);

    // TODO: Add body flag?
    // TODO: Add tags?

    // Boolean Flags
    m_prefix.insert(u"isimportant"_s, u"I"_s);
    m_prefix.insert(u"istoact"_s, u"T"_s);
    m_prefix.insert(u"iswatched"_s, u"W"_s);
    m_prefix.insert(u"isdeleted"_s, u"D"_s);
    m_prefix.insert(u"isspam"_s, u"S"_s);
    m_prefix.insert(u"isreplied"_s, u"E"_s);
    m_prefix.insert(u"isignored"_s, u"G"_s);
    m_prefix.insert(u"isforwarded"_s, u"F"_s);
    m_prefix.insert(u"issent"_s, u"N"_s);
    m_prefix.insert(u"isqueued"_s, u"Q"_s);
    m_prefix.insert(u"isham"_s, u"H"_s);
    m_prefix.insert(u"isread"_s, u"R"_s);
    m_prefix.insert(u"hasattachment"_s, u"A"_s);
    m_prefix.insert(u"isencrypted"_s, u"C"_s);
    m_prefix.insert(u"hasinvitation"_s, u"V"_s);

    m_boolProperties << u"isimportant"_s << u"istoact"_s << QStringLiteral("iswatched") << QStringLiteral("isdeleted") << u"isspam"_s << u"isreplied"_s
                     << QStringLiteral("isignored") << QStringLiteral("isforwarded") << u"issent"_s << u"isqueued"_s << QStringLiteral("isham")
                     << QStringLiteral("isread") << u"hasattachment"_s << u"isencrypted"_s << QStringLiteral("hasinvitation");

    m_valueProperties.insert(u"date"_s, 0);
    m_valueProperties.insert(u"size"_s, 1);
    m_valueProperties.insert(u"onlydate"_s, 2);

    setDbPath(findDatabase(u"email"_s));
}

QStringList EmailSearchStore::types()
{
    return QStringList() << u"Akonadi"_s << u"Email"_s;
}

Xapian::Query EmailSearchStore::constructQuery(const QString &property, const QVariant &value, Term::Comparator com)
{
    // TODO is this special case necessary? maybe we can also move it to PIM
    if (com == Term::Contains) {
        if (!m_prefix.contains(property.toLower())) {
            return {};
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

    const QString subject = QString::fromUtf8(data.c_str(), data.length());
    if (subject.isEmpty()) {
        return u"No Subject"_s;
    }

    return subject;
}

Xapian::Query EmailSearchStore::finalizeQuery(const Xapian::Query &query)
{
    return Xapian::Query(Xapian::Query::OP_AND_MAYBE, query, Xapian::Query(new AgePostingSource(0)));
}

#include "moc_emailsearchstore.cpp"
