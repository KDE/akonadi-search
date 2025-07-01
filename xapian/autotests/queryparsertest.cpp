/*
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "queryparsertest.h"
using namespace Qt::Literals::StringLiterals;

#include "../xapiandatabase.h"
#include "../xapianqueryparser.h"

#include "akonadi_search_xapian_debug.h"
#include <QTemporaryDir>
#include <QTest>

using namespace Akonadi::Search;

void QueryParserTest::testSinglePrefixWord()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery(u"The"_s, u"F"_s);
    Xapian::Query q("Fthe", 1, 1);
    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testSimpleQuery()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery(u"The song of Ice and Fire"_s);

    QList<Xapian::Query> queries;
    queries << Xapian::Query("the", 1, 1);
    queries << Xapian::Query("song", 1, 2);
    queries << Xapian::Query("of", 1, 3);
    queries << Xapian::Query("ice", 1, 4);
    queries << Xapian::Query("and", 1, 5);
    queries << Xapian::Query("fire", 1, 6);

    Xapian::Query q(Xapian::Query::OP_AND, queries.begin(), queries.end());

    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testPhraseSearch()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery(u"The \"song of Ice\" Fire"_s);
    // qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << query.get_description().c_str();

    QList<Xapian::Query> phraseQueries;
    phraseQueries << Xapian::Query("song", 1, 2);
    phraseQueries << Xapian::Query("of", 1, 3);
    phraseQueries << Xapian::Query("ice", 1, 4);

    QList<Xapian::Query> queries;
    queries << Xapian::Query("the", 1, 1);
    queries << Xapian::Query(Xapian::Query::OP_PHRASE, phraseQueries.begin(), phraseQueries.end());
    queries << Xapian::Query("fire", 1, 5);

    Xapian::Query q(Xapian::Query::OP_AND, queries.begin(), queries.end());
    // qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << q.get_description().c_str();
    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testPhraseSearchOnly()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery(u"/opt/pro"_s);
    // qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << query.get_description().c_str();

    QList<Xapian::Query> queries;
    queries << Xapian::Query("opt", 1, 1);
    queries << Xapian::Query("pro", 1, 2);

    Xapian::Query q(Xapian::Query::OP_PHRASE, queries.begin(), queries.end());
    // qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << q.get_description().c_str();
    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testPhraseSearch_sameLimiter()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery(u"The \"song of Ice' and Fire"_s);
    // qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << query.get_description().c_str();

    QList<Xapian::Query> queries;
    queries << Xapian::Query("the", 1, 1);
    queries << Xapian::Query("song", 1, 2);
    queries << Xapian::Query("of", 1, 3);
    queries << Xapian::Query("ice", 1, 4);
    queries << Xapian::Query("and", 1, 5);
    queries << Xapian::Query("fire", 1, 6);

    Xapian::Query q(Xapian::Query::OP_AND, queries.begin(), queries.end());
    // qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << q.get_description().c_str();

    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testPhraseSearchEmail()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery(u"The song@ice.com Fire"_s);

    QList<Xapian::Query> phraseQueries;
    phraseQueries << Xapian::Query("song", 1, 2);
    phraseQueries << Xapian::Query("ice", 1, 3);
    phraseQueries << Xapian::Query("com", 1, 4);

    QList<Xapian::Query> queries;
    queries << Xapian::Query("the", 1, 1);
    queries << Xapian::Query(Xapian::Query::OP_PHRASE, phraseQueries.begin(), phraseQueries.end());
    queries << Xapian::Query("fire", 1, 5);

    Xapian::Query q(Xapian::Query::OP_AND, queries.begin(), queries.end());
    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testAccentSearch()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery(QString::fromUtf8("sóng"));
    Xapian::Query q("song", 1, 1);

    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testUnderscoreSplitting()
{
    XapianQueryParser parser;

    Xapian::Query query = parser.parseQuery(u"The_Fire"_s);

    QList<Xapian::Query> queries;
    queries << Xapian::Query("the", 1, 1);
    queries << Xapian::Query("fire", 1, 2);

    Xapian::Query q(Xapian::Query::OP_AND, queries.begin(), queries.end());

    QCOMPARE(query.serialise(), q.serialise());
}

void QueryParserTest::testWordExpansion()
{
    QTemporaryDir dir;
    XapianDatabase db(dir.path(), true);

    Xapian::Document doc;
    doc.add_term("hell");
    doc.add_term("hello");
    doc.add_term("hellog");
    doc.add_term("hi");
    doc.add_term("hibrid");

    db.replaceDocument(1, doc);
    Xapian::Database *xap = db.db();

    XapianQueryParser parser;
    parser.setDatabase(xap);

    Xapian::Query query = parser.parseQuery(u"hell"_s);
    // qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << query.get_description().c_str();

    QList<Xapian::Query> synQueries;
    synQueries << Xapian::Query("hell", 1, 1);
    synQueries << Xapian::Query("hello", 1, 1);
    synQueries << Xapian::Query("hellog", 1, 1);

    Xapian::Query q(Xapian::Query::OP_SYNONYM, synQueries.begin(), synQueries.end());
    // qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << q.get_description().c_str();

    QCOMPARE(query.serialise(), q.serialise());

    //
    // Try expanding everything
    //
    query = parser.parseQuery(u"hel hi"_s);
    // qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << query.get_description().c_str();

    {
        QList<Xapian::Query> synQueries;
        synQueries << Xapian::Query("hell", 1, 1);
        synQueries << Xapian::Query("hello", 1, 1);
        synQueries << Xapian::Query("hellog", 1, 1);

        Xapian::Query q1(Xapian::Query::OP_SYNONYM, synQueries.begin(), synQueries.end());

        synQueries.clear();
        synQueries << Xapian::Query("hi", 1, 2);
        synQueries << Xapian::Query("hibrid", 1, 2);

        Xapian::Query q2(Xapian::Query::OP_SYNONYM, synQueries.begin(), synQueries.end());

        QList<Xapian::Query> queries;
        queries << q1;
        queries << q2;

        Xapian::Query q(Xapian::Query::OP_AND, queries.begin(), queries.end());
        // qCDebug(AKONADI_SEARCH_XAPIAN_LOG) << q.get_description().c_str();

        QCOMPARE(query.serialise(), q.serialise());
    }

    {
        Xapian::Query query = parser.parseQuery(u"rubbish"_s);
        Xapian::Query q = Xapian::Query("rubbish", 1, 1);

        QCOMPARE(query.serialise(), q.serialise());
    }
}

QTEST_MAIN(QueryParserTest)

#include "moc_queryparsertest.cpp"
