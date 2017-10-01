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

#include "storetest.h"
#include "../src/store.h"
#include "../src/indexer.h"
#include "../src/querymapper.h"
#include "../src/resultiterator.h"

#include <xapian.h>

#include <KContacts/Addressee>
#include <KContacts/ContactGroup>

#include <AkonadiCore/Item>
#include <AkonadiCore/SearchQuery>

#include <QTest>
#include <QStandardPaths>
#include <QVector>

using namespace Akonadi::Search;

Q_DECLARE_METATYPE(Akonadi::SearchQuery)
Q_DECLARE_METATYPE(QVector<Akonadi::Item::Id>)

namespace {

template<typename T>
Akonadi::Item toItem(const T &i, qint64 id, qint64 colId)
{
    Akonadi::Item item(T::mimeType());
    item.setId(id);
    item.setParentCollection(Akonadi::Collection(colId));
    item.setPayload<T>(i);
    return item;
}

}

void StoreTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    indexContacts();
    indexEmails();
    indexIncidences();
    indexNotes();
}

void StoreTest::cleanupTestCase()
{
    QDir dir(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("akonadi/search_db"),
                                    QStandardPaths::LocateDirectory));
    QVERIFY(dir.absolutePath().contains(QLatin1String(".qttest")));
    if (dir.exists()) {
        dir.removeRecursively();
    }
}

void StoreTest::indexContacts()
{
    Akonadi::Item::List items;
    {
        KContacts::Addressee contact;
        contact.setName(QStringLiteral("Daniel Vrátil"));
        contact.setEmails({ QStringLiteral("dvratil@kde.org") });
        contact.setUid(QStringLiteral("1234-ABCD"));
        items << toItem(contact, 1, 1);
    }
    {
        KContacts::Addressee contact;
        contact.setName(QStringLiteral("Daniel Nevrátil"));
        contact.setEmails({ QStringLiteral("nevratil@kde.test") });
        contact.setUid(QStringLiteral("ABCD-1234"));
        items << toItem(contact, 2, 1);
    }
    {
        KContacts::Addressee contact;
        contact.setName(QStringLiteral("Daniel"));
        contact.setEmails({ QStringLiteral("dannyboyl@kde.test") });
        contact.setUid(QStringLiteral("EFGH-IJKL"));
        items << toItem(contact, 3, 1);
    }
    {
        KContacts::Addressee contact;
        contact.setName(QStringLiteral("Indexed Contact"));
        contact.setEmails({ QStringLiteral("indexed-contact@kde.test") });
        contact.setUid(QStringLiteral("0000-0000-0000"));
        items << toItem(contact, 4, 1);
    }

    indexItems(items);
}

void StoreTest::indexEmails()
{
}

void StoreTest::indexIncidences()
{
}

void StoreTest::indexNotes()
{
}

void StoreTest::indexItems(const QVector<Akonadi::Item> &items)
{
    QVERIFY(!items.isEmpty());
    auto indexers = Indexer::forType(items.first().mimeType());
    QCOMPARE(indexers.count(), 1);
    Indexer *indexer = indexers.first();
    QVERIFY(indexer);

    auto stores = Store::getForType(items.first().mimeType());
    QCOMPARE(stores.count(), 1);
    Store *store = stores.first();
    QVERIFY(store);
    store->setOpenMode(Store::WriteOnly);
    QCOMPARE(store->openMode(), Store::WriteOnly);

    for (const auto &item : items) {
        const auto document = indexer->index(item);
        QVERIFY(store->index(item.id(), document));
    }
    QVERIFY(store->commit());

    qDeleteAll(indexers);
    qDeleteAll(stores);
}


void StoreTest::testContactStore_data()
{
    QTest::addColumn<Akonadi::SearchQuery>("akonadiQuery");
    QTest::addColumn<QVector<Akonadi::Item::Id>>("expectedResults");

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Daniel"),
                                              Akonadi::SearchTerm::CondContains));
        QTest::newRow("contains 'Daniel'") << aq << QVector<Akonadi::Item::Id>{ 1, 2, 3 };
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Daniel"),
                                              Akonadi::SearchTerm::CondEqual));
        QTest::newRow("matches 'Daniel'") << aq << QVector<Akonadi::Item::Id>{ 3 };
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Daniel Vrátil"),
                                              Akonadi::SearchTerm::CondEqual));
        QTest::newRow("matches 'Daniel Vrátil'") << aq << QVector<Akonadi::Item::Id>{ 1 };
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Uid, QStringLiteral("ABCD-1234"),
                                              Akonadi::SearchTerm::CondEqual));
        QTest::newRow("matches UID 'ABCD-1234'") << aq << QVector<Akonadi::Item::Id>{ 2 };
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, QStringLiteral("@kde.test"),
                                               Akonadi::SearchTerm::CondContains));
        QTest::newRow("contains '@kde.test'") << aq << QVector<Akonadi::Item::Id>{ 2, 3 ,4 };
    }

    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelOr);
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, QStringLiteral("dvratil@kde.org"),
                                              Akonadi::SearchTerm::CondEqual));
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Contact"),
                                              Akonadi::SearchTerm::CondContains));
        QTest::newRow("matches 'dvratil@kde.org' OR contains 'Contact'") << aq << QVector<Akonadi::Item::Id>{ 1, 4 };
    }
}

void StoreTest::testContactStore()
{
    QFETCH(Akonadi::SearchQuery, akonadiQuery);
    QFETCH(QVector<Akonadi::Item::Id>, expectedResults);

    auto queryMappers = QueryMapper::forType(KContacts::Addressee::mimeType());
    QCOMPARE(queryMappers.count(), 1);
    QueryMapper *mapper = queryMappers.first();
    QVERIFY(mapper);

    auto stores = Store::getForType(KContacts::Addressee::mimeType());
    QCOMPARE(stores.count(), 1);
    Store *store = stores.first();
    QVERIFY(store);
    store->setOpenMode(Store::ReadOnly);

    const auto query = mapper->map(akonadiQuery);
    auto it = store->search(query);
    QVector<Akonadi::Item::Id> ids;
    while (it.next()) {
        ids << it.id();
    }

    qSort(ids); // easier to compare, we don't care about the order here
    QCOMPARE(ids, expectedResults);

    qDeleteAll(stores);
    qDeleteAll(queryMappers);
}

void StoreTest::testEmailStore_data()
{
}

void StoreTest::testEmailStore()
{
}

void StoreTest::testCalendarStore_data()
{
}

void StoreTest::testCalendarStore()
{
}

void StoreTest::testNoteStore_data()
{
}

void StoreTest::testNoteStore()
{
}

QTEST_MAIN(StoreTest)
