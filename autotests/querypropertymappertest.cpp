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

#include "querypropertymappertest.h"

#include "../src/querypropertymapper_p.h"

#include <QTest>

using namespace Akonadi::Search;

class TestQueryPropertyMapper : public QueryPropertyMapper
{
public:
    explicit TestQueryPropertyMapper()
        : QueryPropertyMapper()
    {
    }
};

void QueryPropertyMapperTest::testPrefix()
{
    TestQueryPropertyMapper mapper;

    mapper.insertPrefix(1, QStringLiteral("prefix"));
    QVERIFY(mapper.hasPrefix(1));
    QCOMPARE(mapper.prefix(1), std::string("prefix"));

    QVERIFY(!mapper.hasPrefix(2));
}

void QueryPropertyMapperTest::testBoolProperty()
{
    TestQueryPropertyMapper mapper;

    mapper.insertBoolProperty(1);
    QVERIFY(mapper.hasBoolProperty(1));
}

void QueryPropertyMapperTest::testBoolValueProperty()
{
    TestQueryPropertyMapper mapper;

    mapper.insertBoolValueProperty(1);
    QVERIFY(mapper.hasBoolValueProperty(1));

    QVERIFY(!mapper.hasBoolValueProperty(2));
}

void QueryPropertyMapperTest::testValueProperty()
{
    TestQueryPropertyMapper mapper;

    mapper.insertValueProperty(3, 42);
    QVERIFY(mapper.hasValueProperty(3));
    QCOMPARE(mapper.valueProperty(3), 42);

    QVERIFY(!mapper.hasValueProperty(4));
}

QTEST_MAIN(QueryPropertyMapperTest)
