/*
 * Copyright (C) 2017  Daniel Vrátil <dvratil@kde.org>
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

#include "querymappertest.h"
#include "../src/querymapper.h"

#include <AkonadiCore/SearchQuery>
#include <Akonadi/KMime/MessageFlags>

#include <QTest>
#include <QDebug>

#include <xapian.h>

Q_DECLARE_METATYPE(Akonadi::SearchQuery)
Q_DECLARE_METATYPE(Xapian::Query)

using namespace Akonadi::Search;

namespace {

static const int XapianMatches  = Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PHRASE;
static const int XapianContains = Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL;

}

void QueryMapperTest::testQueryMapper(const QString &mimeType)
{
    QFETCH(Akonadi::SearchQuery, akonadiQuery);
    QFETCH(Xapian::Query, xapianQuery);

    const auto mappers = QueryMapper::forType(mimeType);
    QCOMPARE(mappers.count(), 1);

    const auto result = mappers.first()->map(akonadiQuery);
    //qDebug() << result.get_description().c_str();
    //qDebug() << xapianQuery.get_description().c_str();
    const auto expectedSerialized = xapianQuery.serialise();
    QCOMPARE(result, QByteArray(expectedSerialized.c_str(), expectedSerialized.size()));

    qDeleteAll(mappers);
}

void QueryMapperTest::testContactQueryMapper_data()
{
    QTest::addColumn<Akonadi::SearchQuery>("akonadiQuery");
    QTest::addColumn<Xapian::Query>("xapianQuery");

    Xapian::QueryParser parser;

    // TODO: We should genuinely have more tests...
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Daniel")));
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, QStringLiteral("kde.org"),
                                              Akonadi::SearchTerm::CondContains));
        const auto phr = { Xapian::Query("NA^", 1 ,1), Xapian::Query("NAdaniel", 1, 1), Xapian::Query("NA$", 1, 2) };
        const auto xsq = { Xapian::Query(Xapian::Query::OP_PHRASE, phr.begin(), phr.end()),
                           parser.parse_query("kde.org", XapianContains, "")
                         };
        Xapian::Query xq(Xapian::Query::OP_AND, xsq.begin(), xsq.end());

        QTest::newRow("name matches AND email contains") << aq << xq;
    }

    {
        Akonadi::SearchQuery aq;
        Akonadi::ContactSearchTerm term(Akonadi::ContactSearchTerm::Name, QStringLiteral("Daniel"),
                                        Akonadi::ContactSearchTerm::CondContains);
        term.setIsNegated(true);
        aq.addTerm(term);

        const auto xq = parser.parse_query("Daniel", XapianContains, "NA");
        QTest::newRow("all contains")
            << aq << Xapian::Query(Xapian::Query::OP_AND_NOT, Xapian::Query::MatchAll, xq);
    }
}

void QueryMapperTest::testContactQueryMapper()
{
    testQueryMapper(QStringLiteral("text/directory"));
}



void QueryMapperTest::testEmailQueryMapper_data()
{
    QTest::addColumn<Akonadi::SearchQuery>("akonadiQuery");
    QTest::addColumn<Xapian::Query>("xapianQuery");

    Xapian::QueryParser parser;

    // TODO: We should genuinely have more tests...
    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelOr);
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("hello"), Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("test")));

        const auto phr = { Xapian::Query("SU^", 1, 1), Xapian::Query("SUtest", 1, 1), Xapian::Query("SU$", 1, 2) };
        const auto xsq = { parser.parse_query("hello", XapianContains, "BO"),
                           Xapian::Query(Xapian::Query::OP_PHRASE, phr.begin(), phr.end()) };
        QTest::newRow("body contains OR subject matches")
            << aq << Xapian::Query(Xapian::Query::OP_OR, xsq.begin(), xsq.end());
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, 1024, Akonadi::SearchTerm::CondGreaterOrEqual));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QLatin1String(Akonadi::MessageFlags::HasAttachment)));

        const auto xsq = { Xapian::Query(Xapian::Query::OP_VALUE_GE, 1, Xapian::sortable_serialise(1024)),
                           Xapian::Query("BA") };
        QTest::newRow("size GE AND is attachment")
            << aq << Xapian::Query(Xapian::Query::OP_AND, xsq.begin(), xsq.end());
    }
}

void QueryMapperTest::testEmailQueryMapper()
{
    testQueryMapper(QStringLiteral("message/rfc822"));
}

void QueryMapperTest::testIncidenceQueryMapper_data()
{
    QTest::addColumn<Akonadi::SearchQuery>("akonadiQuery");
    QTest::addColumn<Xapian::Query>("xapianQuery");

    Xapian::QueryParser parser;

    // TODO: We should genuinely have more tests...
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Summary, QStringLiteral("Meeting"), Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Organizer, QStringLiteral("dvratil@kde.org"), Akonadi::SearchTerm::CondEqual));

        const auto phr = { Xapian::Query("O^", 1, 1), Xapian::Query("Odvratil", 1, 1),
                           Xapian::Query("Okde", 1, 2), Xapian::Query("Oorg", 1, 3),
                           Xapian::Query("O$", 1, 5) };
        const auto xsq = { parser.parse_query("Meeting", XapianContains, "S"),
                           Xapian::Query(Xapian::Query::OP_PHRASE, phr.begin(), phr.end()) };
        QTest::newRow("summary contains AND organizer matches")
            << aq << Xapian::Query(Xapian::Query::OP_AND, xsq.begin(), xsq.end());
    }
}

void QueryMapperTest::testIncidenceQueryMapper()
{
    testQueryMapper(QStringLiteral("application/x-vnd.akonadi.calendar.event"));
}

void QueryMapperTest::testNotesQueryMapper_data()
{
    QTest::addColumn<Akonadi::SearchQuery>("akonadiQuery");
    QTest::addColumn<Xapian::Query>("xapianQuery");

    Xapian::QueryParser parser;

    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelOr);
        aq.addTerm(QStringLiteral("subject"), QStringLiteral("A Note"), Akonadi::SearchTerm::CondEqual);
        aq.addTerm(QStringLiteral("body"), QStringLiteral("A Note"), Akonadi::SearchTerm::CondContains);

        const auto phr = { Xapian::Query("SU^", 1, 1), Xapian::Query("SUa", 1, 1),
                           Xapian::Query("SUnote", 1, 2), Xapian::Query("SU$", 1, 4) };
        const auto xsq = { Xapian::Query(Xapian::Query::OP_PHRASE, phr.begin(), phr.end()),
                           parser.parse_query("A Note", XapianContains, "BO") };
        QTest::newRow("subject matches OR body contains")
            << aq << Xapian::Query(Xapian::Query::OP_OR, xsq.begin(), xsq.end());
    }

    {
        Akonadi::SearchQuery aq;
        Akonadi::SearchTerm term(QStringLiteral("subject"), QStringLiteral("The Note"), Akonadi::SearchTerm::CondEqual);
        term.setIsNegated(true);
        aq.addTerm(term);

        const auto phr = { Xapian::Query("SU^", 1, 1), Xapian::Query("SUthe", 1, 1),
                           Xapian::Query("SUnote", 1, 2), Xapian::Query("SU$", 1, 4) };
        const auto xq = Xapian::Query(Xapian::Query::OP_PHRASE, phr.begin(), phr.end());
        QTest::newRow("subject not matches")
            << aq << Xapian::Query(Xapian::Query::OP_AND_NOT, Xapian::Query::MatchAll, xq);
    }
}

void QueryMapperTest::testNotesQueryMapper()
{
    testQueryMapper(QStringLiteral("text/x-vnd.akonadi.note"));
}




QTEST_MAIN(QueryMapperTest)
