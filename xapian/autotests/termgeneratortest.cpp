/*
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "termgeneratortest.h"
#include "../xapiandatabase.h"
#include "../xapiantermgenerator.h"

#include "akonadi_search_xapian_debug.h"
#include <QTemporaryDir>
#include <QTest>

using namespace Akonadi::Search;

namespace
{
QStringList allWords(const Xapian::Document &doc)
{
    QStringList words;
    for (auto it = doc.termlist_begin(); it != doc.termlist_end(); it++) {
        std::string str = *it;
        words << QString::fromUtf8(str.c_str(), str.length());
    }

    return words;
}
}
void TermGeneratorTest::testWordBoundaries()
{
    QString str = QStringLiteral("The quick (\"brown\") 'fox' can't jump 32.3 feet, right? No-Wrong;xx.txt");

    Xapian::Document doc;
    XapianTermGenerator termGen(&doc);
    termGen.indexText(str);

    QStringList words = allWords(doc);

    QStringList expectedWords;
    expectedWords << QStringLiteral("32.3") << QStringLiteral("brown") << QStringLiteral("can't") << QStringLiteral("feet") << QStringLiteral("fox")
                  << QStringLiteral("jump") << QStringLiteral("no") << QStringLiteral("quick") << QStringLiteral("right") << QStringLiteral("the")
                  << QStringLiteral("txt") << QStringLiteral("wrong") << QStringLiteral("xx");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testUnderscore_splitting()
{
    QString str = QStringLiteral("Hello_Howdy");

    Xapian::Document doc;
    XapianTermGenerator termGen(&doc);
    termGen.indexText(str);

    const auto aW = allWords(doc);
    const auto words = QSet<QString>(aW.constBegin(), aW.constEnd());

    QSet<QString> expectedWords;
    expectedWords << QStringLiteral("hello") << QStringLiteral("howdy") << QStringLiteral("hello_howdy");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testAccetCharacters()
{
    QString str = QString::fromUtf8("Como está Kûg");

    Xapian::Document doc;
    XapianTermGenerator termGen(&doc);
    termGen.indexText(str);

    const auto aW = allWords(doc);
    const auto words = QSet<QString>(aW.constBegin(), aW.constEnd());

    QSet<QString> expectedWords;
    expectedWords << QStringLiteral("como") << QStringLiteral("esta") << QStringLiteral("kug") << QString::fromUtf8("está") << QString::fromUtf8("kûg");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testUnicodeCompatibleComposition()
{
    // The 0xfb00 corresponds to U+FB00 which is a 'ff'
    QString str = QStringLiteral("maffab");
    QString str2 = QLatin1String("ma") + QChar(0xfb00) + QStringLiteral("ab");

    Xapian::Document doc;
    XapianTermGenerator termGen(&doc);
    termGen.indexText(str2);

    QStringList words = allWords(doc);
    QStringList expectedWords({str, str2});
    QCOMPARE(words, expectedWords);

    QString output = words.first();
    QCOMPARE(str, output);
}

void TermGeneratorTest::testEmails()
{
    QString str = QStringLiteral("me@vhanda.in");

    Xapian::Document doc;
    XapianTermGenerator termGen(&doc);
    termGen.indexText(str);

    QStringList words = allWords(doc);

    QStringList expectedWords;
    expectedWords << QStringLiteral("in") << QStringLiteral("me") << QStringLiteral("vhanda");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testWordPositions()
{
    QTemporaryDir dir;
    XapianDatabase db(dir.path(), true);

    Xapian::Document doc;
    XapianTermGenerator termGen(&doc);

    QString str = QStringLiteral("Hello hi how hi");
    termGen.indexText(str);

    db.replaceDocument(1, doc);

    Xapian::Database *xap = db.db();

    Xapian::PositionIterator it = xap->positionlist_begin(1, "hello");
    Xapian::PositionIterator end = xap->positionlist_end(1, "hello");
    QVERIFY(it != end);
    QCOMPARE(*it, (uint)1);
    it++;
    QVERIFY(it == end);

    it = xap->positionlist_begin(1, "hi");
    end = xap->positionlist_end(1, "hi");
    QVERIFY(it != end);
    QCOMPARE(*it, (uint)2);
    it++;
    QCOMPARE(*it, (uint)4);
    it++;
    QVERIFY(it == end);

    it = xap->positionlist_begin(1, "how");
    end = xap->positionlist_end(1, "how");
    QVERIFY(it != end);
    QCOMPARE(*it, (uint)3);
    it++;
    QVERIFY(it == end);
}

QTEST_MAIN(TermGeneratorTest)

#include "moc_termgeneratortest.cpp"
