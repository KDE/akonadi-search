/*
 * Copyright (C) 2018  Daniel Vrátil <dvratil@kde.org>
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

#include "contactcompletertest.h"
#include "../src/utils.h"
#include "../src/contactcompleter.h"
#include "../src/store.h"
#include "../src/indexer.h"

#include <xapian.h>

#include <KMime/Message>

#include <AkonadiCore/Item>

#include <QStandardPaths>
#include <QTest>
#include <QSignalSpy>

using namespace Akonadi::Search;

namespace {

template<typename T>
Akonadi::Item toItem(const QSharedPointer<T> &i, qint64 id, qint64 colId)
{
    Akonadi::Item item(T::mimeType());
    item.setId(id);
    item.setParentCollection(Akonadi::Collection(colId));
    item.setPayload<QSharedPointer<T>>(i);
    return item;
}

}

void ContactCompleterTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    // Index a couple of addresses
    Akonadi::Item::List items;
    {
        auto email = KMime::Message::Ptr::create();
        email->from(true)->fromUnicodeString(QStringLiteral("Daniel Vrátil <dvratil@kde.org>"), "UTF-8");
        email->to(true)->from7BitString("Indexed Contact <indexed-contact@kde.test>");
        email->assemble();
        auto item = toItem(email, 100, 3);
        items << item;
    }

    {
        auto email = KMime::Message::Ptr::create();
        email->from(true)->fromUnicodeString(QStringLiteral("Daniel Nevrátil <nevratil@kde.test>"), "UTF-8");
        email->to(true)->from7BitString("Indexed Contact <indexed-contact@kde.test>");
        email->cc(true)->fromUnicodeString(QStringLiteral("Daniel Vrátil <dvratil@kde.org>"), "UTF-8");
        email->assemble();
        auto item = toItem(email, 101, 3);
        items << item;
    }

    {
        auto email = KMime::Message::Ptr::create();
        email->from(true)->fromUnicodeString(QStringLiteral("Rezervační systém <no-reply@moje.hospudka>"), "UTF-8");
        email->to(true)->from7BitString("John Doe <john@doe.example>");
        email->assemble();
        auto item = toItem(email, 102, 3);
        items << item;
    }
    {
        KMime::Message::Ptr msg(new KMime::Message);
        msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
        msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
        msg->assemble();
        auto item = toItem(msg, 103, 4);
        items << item;
    }
    {
        KMime::Message::Ptr msg(new KMime::Message);
        msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
        msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
        msg->assemble();

        auto item = toItem(msg, 104, 5);
        items << item;
    }
    {
        KMime::Message::Ptr msg(new KMime::Message);
        msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
        msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
        msg->assemble();

        auto item = toItem(msg, 105, 5);
        items << item;
    }
    {
        KMime::Message::Ptr msg(new KMime::Message);
        msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
        msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
        msg->cc()->addAddress("cc@test.com", QStringLiteral("Jane Doe"));
        msg->bcc()->addAddress("bcc@test.com", QStringLiteral("Jane Doe"));
        msg->assemble();

        auto item = toItem(msg, 106, 5);
        items << item;
    }

    QScopedPointer<Indexer> indexer(Indexer::create(EmailContactsMimeType()));
    QVERIFY(indexer);
    QVERIFY(!items.isEmpty());
    QScopedPointer<Store> store(Store::create(EmailContactsMimeType()));
    store->setOpenMode(Store::WriteOnly);
    QCOMPARE(store->openMode(), Store::WriteOnly);
    for (const auto &item : items) {
        const auto document = indexer->index(item, {});
        QVERIFY(store->index(item.id(), document));
    }
    QVERIFY(store->commit());
}

void ContactCompleterTest::cleanupTestCase()
{
    QDir dir(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("akonadi/search_db"),
                                    QStandardPaths::LocateDirectory));
    QVERIFY(dir.absolutePath().contains(QLatin1String(".qttest")));
    if (dir.exists()) {
       dir.removeRecursively();
    }
}

void ContactCompleterTest::testContactCompleter_data()
{
    QTest::addColumn<QString>("query");
    QTest::addColumn<QStringList>("expectedResults");

    QTest::newRow("Daniel")
        << QStringLiteral("Daniel")
        << QStringList{ QStringLiteral("Daniel Vrátil <dvratil@kde.org>"),
                        QStringLiteral("Daniel Nevrátil <nevratil@kde.test>") };
    QTest::newRow("test")
        << QStringLiteral("test")
        << QStringList{ QStringLiteral("Indexed Contact <indexed-contact@kde.test>"),
                        QStringLiteral("Daniel Nevrátil <nevratil@kde.test>"),
                        QStringLiteral("John Doe <john@test.com>"),
                        QStringLiteral("Jane Doe <jane@test.com>"),
                        QStringLiteral("Jane Doe <cc@test.com>"),
                        QStringLiteral("Jane Doe <bcc@test.com>") };
    QTest::newRow("Jane")
        << QStringLiteral("jane")
        << QStringList{ QStringLiteral("Jane Doe <jane@test.com>"),
                        QStringLiteral("Jane Doe <cc@test.com>"),
                        QStringLiteral("Jane Doe <bcc@test.com>") };
    QTest::newRow("doe")
        << QStringLiteral("doe")
        << QStringList{ QStringLiteral("John Doe <john@test.com>"),
                        QStringLiteral("John Doe <john@doe.example>"),
                        QStringLiteral("Jane Doe <jane@test.com>"),
                        QStringLiteral("Jane Doe <cc@test.com>"),
                        QStringLiteral("Jane Doe <bcc@test.com>") };
    QTest::newRow("kde.test")
        << QStringLiteral("kde.test")
        << QStringList{ QStringLiteral("Daniel Nevrátil <nevratil@kde.test>"),
                        QStringLiteral("Indexed Contact <indexed-contact@kde.test>") };
}

void ContactCompleterTest::testContactCompleter()
{
    QFETCH(QString, query);
    QFETCH(QStringList, expectedResults);

    ContactCompleter completer(query, 100);
    completer.setAutoDelete(false);
    QSignalSpy spy(&completer, &ContactCompleter::finished);
    completer.start();
    QVERIFY(spy.wait(100));

    QCOMPARE(spy.count(), 1);
    auto results = spy.first().first().toStringList();
    qSort(results);
    qSort(expectedResults);
    qDebug() << results;
    qDebug() << expectedResults;
    QCOMPARE(results, expectedResults);
}

QTEST_GUILESS_MAIN(ContactCompleterTest)
