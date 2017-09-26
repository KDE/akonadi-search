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

void QueryPropertyMapperTest::testPrefix()
{
    QueryPropertyMapper mapper;

    mapper.insertPrefix(QStringLiteral("property"), QStringLiteral("prefix"));
    QVERIFY(mapper.hasPrefix(QStringLiteral("property")));
    QCOMPARE(mapper.prefix(QStringLiteral("property")), std::string("prefix"));

    QVERIFY(!mapper.hasPrefix(QStringLiteral("anotherProperty")));
}

void QueryPropertyMapperTest::testBoolProperty()
{
    QueryPropertyMapper mapper;

    mapper.insertBoolProperty(QStringLiteral("boolProperty"));
    QVERIFY(mapper.hasBoolProperty(QStringLiteral("boolProperty")));
}

void QueryPropertyMapperTest::testBoolValueProperty()
{
    QueryPropertyMapper mapper;

    mapper.insertBoolValueProperty(QStringLiteral("boolValueProperty"));
    QVERIFY(mapper.hasBoolValueProperty(QStringLiteral("boolValueProperty")));

    QVERIFY(!mapper.hasBoolValueProperty(QStringLiteral("otherBoolValueProperty")));
}

void QueryPropertyMapperTest::testValueProperty()
{
    QueryPropertyMapper mapper;

    mapper.insertValueProperty(QStringLiteral("valueProperty"), 42);
    QVERIFY(mapper.hasValueProperty(QStringLiteral("valueProperty")));
    QCOMPARE(mapper.valueProperty(QStringLiteral("valueProperty")), 42);

    QVERIFY(!mapper.hasValueProperty(QStringLiteral("otherValueProperty")));
}

QTEST_MAIN(QueryPropertyMapperTest)
