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
#include <KMime/Message>
#include <KCalCore/Event>
#include <KCalCore/Todo>

#include <Akonadi/KMime/MessageFlags>

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
Akonadi::Item toItem(const T &i, qint64 id, qint64 colId, const QString &mimeType)
{
    Akonadi::Item item(mimeType);
    item.setId(id);
    item.setParentCollection(Akonadi::Collection(colId));
    item.setPayload<T>(i);
    return item;
}


template<typename T>
Akonadi::Item toItem(const QSharedPointer<T> &i, qint64 id, qint64 colId)
{
    return toItem<QSharedPointer<T>>(i, id, colId, T::mimeType());
}

template<typename T>
Akonadi::Item toItem(const T &i, qint64 id, qint64 colId)
{
    return toItem<T>(i, id, colId, T::mimeType());
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

void StoreTest::indexItems(const QVector<Akonadi::Item> &items)
{
    for (const auto &item : items) {
        QVERIFY(!items.isEmpty());
        auto indexers = Indexer::create(item.mimeType());
        QCOMPARE(indexers.count(), 1);
        Indexer *indexer = indexers.first();
        QVERIFY(indexer);

        auto stores = Store::create(item.mimeType());
        QCOMPARE(stores.count(), 1);
        Store *store = stores.first();
        QVERIFY(store);
        store->setOpenMode(Store::WriteOnly);
        QCOMPARE(store->openMode(), Store::WriteOnly);

        const auto document = indexer->index(item);
        QVERIFY(store->index(item.id(), document));
        QVERIFY(store->commit());

        qDeleteAll(indexers);
        qDeleteAll(stores);
    }
}

void StoreTest::testStore()
{
    QFETCH(Akonadi::SearchQuery, akonadiQuery);
    QFETCH(QVector<Akonadi::Item::Id>, expectedResults);
    QFETCH(QString, mimeType);

    auto queryMappers = QueryMapper::create(mimeType);
    QCOMPARE(queryMappers.count(), 1);
    QueryMapper *mapper = queryMappers.first();
    QVERIFY(mapper);

    auto stores = Store::create(mimeType);
    QCOMPARE(stores.count(), 1);
    Store *store = stores.first();
    QVERIFY(store);
    QCOMPARE(store->openMode(), Store::ReadOnly);

    const auto query = mapper->map(akonadiQuery);
    QVERIFY(!query.isEmpty());
    //qDebug() << query.get_description().c_str();
    auto it = store->search(query);
    QVector<Akonadi::Item::Id> ids;
    while (it.next()) {
        ids << it.id();
    }

    qSort(ids); // easier to compare, we don't care about the order here
    //qDebug() << ids;
    //qDebug() << expectedResults;
    QCOMPARE(ids, expectedResults);

    qDeleteAll(stores);
    qDeleteAll(queryMappers);
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
    {
        KContacts::Addressee contact;
        contact.setUid(QStringLiteral("uid1"));
        contact.setName(QStringLiteral("John Doe"));
        contact.setFormattedName(QStringLiteral("John Doe"));
        contact.setNickName(QStringLiteral("JD"));
        contact.setEmails(QStringList() << QStringLiteral("john@test.com"));
        contact.setBirthday(QDateTime(QDate(2000, 01, 01)));
        items << toItem(contact, 5, 1);
    }
    {
        KContacts::Addressee contact;
        contact.setUid(QStringLiteral("uid2"));
        contact.setName(QStringLiteral("Jane Doe"));
        contact.setEmails(QStringList() << QStringLiteral("jane@test.com"));
        contact.setBirthday(QDateTime(QDate(2001, 01, 01)));
        items << toItem(contact, 6, 1);
    }
    {
        KContacts::Addressee contact;
        contact.setUid(QStringLiteral("uid2"));
        contact.setName(QStringLiteral("Jane Doe"));
        contact.setEmails(QStringList() << QStringLiteral("JANE@TEST.COM"));
        contact.setBirthday(QDateTime(QDate(2001, 01, 01)));
        items << toItem(contact, 7, 1);
    }
    {
        KContacts::Addressee contact;
        contact.setUid(QStringLiteral("abcd-efgh-1234-5678"));
        contact.setName(QStringLiteral("Dan Vrátil"));
        contact.setEmails({ QStringLiteral("dan@test.com") });
        contact.setBirthday(QDateTime(QDate(2002, 01, 01)));
        items << toItem(contact, 8, 1);
    }
    {
        KContacts::ContactGroup group;
        group.setName(QStringLiteral("group1"));
        items << toItem(group, 9, 1);
    }
    {
        KContacts::ContactGroup group;
        group.setName(QStringLiteral("group3"));
        items << toItem(group, 10, 2);
    }


    indexItems(items);
}

void StoreTest::testContactStore_data()
{
    QTest::addColumn<Akonadi::SearchQuery>("akonadiQuery");
    QTest::addColumn<QVector<Akonadi::Item::Id>>("expectedResults");
    QTest::addColumn<QString>("mimeType");

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Daniel"),
                                              Akonadi::SearchTerm::CondContains));
        QTest::newRow("contains 'Daniel'") << aq << QVector<Akonadi::Item::Id>{ 1, 2, 3 }
                                           << KContacts::Addressee::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Daniel"),
                                              Akonadi::SearchTerm::CondEqual));
        QTest::newRow("matches 'Daniel'") << aq << QVector<Akonadi::Item::Id>{ 3 }
                                          << KContacts::Addressee::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Daniel Vrátil"),
                                              Akonadi::SearchTerm::CondEqual));
        QTest::newRow("matches 'Daniel Vrátil'") << aq << QVector<Akonadi::Item::Id>{ 1 }
                                                 << KContacts::Addressee::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Uid, QStringLiteral("ABCD-1234"),
                                              Akonadi::SearchTerm::CondEqual));
        QTest::newRow("matches UID 'ABCD-1234'") << aq << QVector<Akonadi::Item::Id>{ 2 }
                                                 << KContacts::Addressee::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, QStringLiteral("@kde.test"),
                                               Akonadi::SearchTerm::CondContains));
        QTest::newRow("contains '@kde.test'") << aq << QVector<Akonadi::Item::Id>{ 2, 3 ,4 }
                                              << KContacts::Addressee::mimeType();
    }

    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelOr);
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, QStringLiteral("dvratil@kde.org"),
                                              Akonadi::SearchTerm::CondEqual));
        aq.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Contact"),
                                              Akonadi::SearchTerm::CondContains));
        QTest::newRow("matches 'dvratil@kde.org' OR contains 'Contact'") << aq << QVector<Akonadi::Item::Id>{ 1, 4 }
                                                                         << KContacts::Addressee::mimeType();
    }

    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("John"),
                                                 Akonadi::SearchTerm::CondContains));
        QTest::newRow("contact by name") << query << QVector<Akonadi::Item::Id>{ 5 }
                                         << KContacts::Addressee::mimeType();
    }

#if 0 // TODO
    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("John"),
                                                 Akonadi::SearchTerm::CondContains));
        QVector<qint64> collections = QVector<qint64>() << 4;
        QTest::newRow("contact collectionfilter") << query << QVector<Akonadi::Item::Id>{};
    }
#endif
    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("john"),
                                                 Akonadi::SearchTerm::CondContains));
        QTest::newRow("contact by lowercase name") << query << QVector<Akonadi::Item::Id>{ 5 }
                                                   << KContacts::Addressee::mimeType();
    }

    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Nickname, QStringLiteral("JD"),
                                                 Akonadi::SearchTerm::CondContains));
        QTest::newRow("contact by nickname") << query << QVector<Akonadi::Item::Id>{ 5 }
                                             << KContacts::Addressee::mimeType();
    }

    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Uid, QStringLiteral("uid1"),
                                                 Akonadi::SearchTerm::CondEqual));
        QTest::newRow("contact by uid") << query << QVector<Akonadi::Item::Id>{ 5 }
                                        << KContacts::Addressee::mimeType();
    }

    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Uid, QStringLiteral("abcd-efgh-1234-5678")));
        QTest::newRow("contact by uid 2") << query << QVector<Akonadi::Item::Id>{ 8 }
                                          << KContacts::Addressee::mimeType();
    }

    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, QStringLiteral("JANE@TEST.COM")));
        QTest::newRow("contact by email") << query << QVector<Akonadi::Item::Id>{ 6, 7 }
                                          << KContacts::Addressee::mimeType();
    }

    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Doe"),
                                                 Akonadi::SearchTerm::CondContains));
        QTest::newRow("contact by name (Doe)") << query << QVector<Akonadi::Item::Id>{ 5, 6, 7 }
                                               << KContacts::Addressee::mimeType();
    }

    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Do"),
                                                 Akonadi::SearchTerm::CondContains));
        QTest::newRow("contact by name (Do)") << query << QVector<Akonadi::Item::Id>{ 5, 6, 7 }
                                              << KContacts::Addressee::mimeType();
    }

    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("group1"),
                                                 Akonadi::SearchTerm::CondContains));
        query.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 1ll));
        QTest::newRow("contact group by name (group1)") << query << QVector<Akonadi::Item::Id>{ 9 }
                                                        << KContacts::ContactGroup::mimeType();
    }

    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("group2"),
                                                 Akonadi::SearchTerm::CondContains));
        query.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 1ll));
        QTest::newRow("contact group by name (group2)") << query << QVector<Akonadi::Item::Id>{}
                                                        << KContacts::ContactGroup::mimeType();
    }

    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("group3"),
                                                 Akonadi::SearchTerm::CondContains));
        query.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 2ll));
        QTest::newRow("contact group by name (group3 in collection 2)") << query << QVector<Akonadi::Item::Id>{ 10 }
                                                                        << KContacts::ContactGroup::mimeType();
    }
    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("group3"),
                                                 Akonadi::SearchTerm::CondContains));
        query.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 1ll));
        QTest::newRow("contact group by name (group3 in collection 1)") << query << QVector<Akonadi::Item::Id>{}
                                                                        << KContacts::ContactGroup::mimeType();
    }
    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Birthday, QDate(2000, 1, 1)));
        QTest::newRow("contact by birthday") << query << QVector<Akonadi::Item::Id>{ 5 }
                                             << KContacts::Addressee::mimeType();
    }
    {
        Akonadi::SearchQuery query;
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Birthday, QDate(2001, 1, 1),
                                                 Akonadi::SearchTerm::CondGreaterOrEqual));
        query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Birthday, QDate(2001, 12, 12),
                                                 Akonadi::SearchTerm::CondLessOrEqual));
        QTest::newRow("contact by birthday range") << query << QVector<Akonadi::Item::Id>{ 6, 7 }
                                                   << KContacts::Addressee::mimeType();
    }
}

void StoreTest::testContactStore()
{
    testStore();
}

void StoreTest::indexEmails()
{
    Akonadi::Item::List items;
    {
        auto email = KMime::Message::Ptr::create();
        email->subject(true)->from7BitString("Test message");
        email->from(true)->fromUnicodeString(QStringLiteral("Daniel Vrátil <dvratil@kde.org>"), "UTF-8");
        email->to(true)->from7BitString("Indexed Contact <indexed-contact@kde.test>");
        email->date(true)->setDateTime({ { 2017, 10, 2 }, { 15, 12, 23 } });
        email->contentType(true)->from7BitString("text/plain");
        email->setBody("Hi there,\n\nThis is done. This is an important email about plumbus");
        email->assemble();
        auto item = toItem(email, 100, 3);
        item.setSize(20);
        item.setFlags({ Akonadi::MessageFlags::Seen, Akonadi::MessageFlags::Flagged });
        items << item;
    }

    {
        auto email = KMime::Message::Ptr::create();
        email->subject(true)->from7BitString("Another Test Message");
        email->from(true)->fromUnicodeString(QStringLiteral("Daniel Nevrátil <nevratil@kde.test>"), "UTF-8");
        email->to(true)->from7BitString("Indexed Contact <indexed-contact@kde.test>");
        email->cc(true)->fromUnicodeString(QStringLiteral("Daniel Vrátil <dvratil@kde.org>"), "UTF-8");
        email->date(true)->setDateTime({ { 2017, 10, 2 }, { 10, 0, 0 } });
        email->contentType(true)->from7BitString("text/html");
        email->setBody("<html><head></head></body>"
                       "<p>Hi guys,</p>"
                       "<p>How are you? This is an <b>HTML</b> message! Be careful with it!</p>"
                       "</body></html>");
        email->assemble();
        auto item = toItem(email, 101, 3);
        item.setSize(21);
        item.setFlags({ Akonadi::MessageFlags::Seen, Akonadi::MessageFlags::HasAttachment });
        items << item;
    }

    {
        auto email = KMime::Message::Ptr::create();
        email->subject(true)->fromUnicodeString(QStringLiteral("Vaše rezervace"), "UTF-8");
        email->from(true)->fromUnicodeString(QStringLiteral("Rezervační systém <no-reply@moje.hospudka>"), "UTF-8");
        email->to(true)->from7BitString("John Doe <john@doe.example>");
        email->date(true)->setDateTime({ { 2016, 5, 25 }, { 14, 43, 33 } });
        email->contentType(true)->setMimeType("text/plain");
        email->contentType(true)->setCharset("UTF-8");
        email->contentTransferEncoding(true)->setEncoding(KMime::Headers::CEquPr);
        email->contentTransferEncoding(true)->setDecoded(false);
        email->setBody("Dobr=C3=BD den,\n\n"
                       "potvrzujeme Va=C5=A1i rezervaci do na=C5=A1=C3=AD hosp=C5=AFdky na =C4=8Dtv=\n"
                       "rtek od 18:00 hodin a t=C4=9B=C5=A1=C3=ADme se na v=C3=A1s.\n\n"
                       "S pozdravem,\n"
                       "na=C5=A1=C3=AD hosp=C5=AFdka\n");
        email->assemble();
        auto item = toItem(email, 102, 3);
        item.setSize(101);
        items << item;
    }
    {
        KMime::Message::Ptr msg(new KMime::Message);
        msg->subject()->from7BitString("subject1");
        msg->contentType()->setMimeType("text/plain");
        msg->contentType()->setCharset("utf-8");
        msg->setBody("body1 mälmöö");
        msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
        msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
        msg->date()->setDateTime(QDateTime(QDate(2013, 11, 10), QTime(12, 0, 0)));
        msg->assemble();
        auto item = toItem(msg, 103, 4);
        item.setSize(1000);
        item.setFlags({ Akonadi::MessageFlags::Replied, Akonadi::MessageFlags::Encrypted });
        items << item;
    }
    {
        KMime::Message::Ptr msg(new KMime::Message);
        msg->subject()->from7BitString("subject2");

        //Multipart message
        KMime::Content *b = new KMime::Content;
        b->contentType()->setMimeType("text/plain");
        b->setBody("body2");
        msg->addContent(b, true);

        msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
        msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
        msg->date()->setDateTime(QDateTime(QDate(2013, 11, 10), QTime(13, 0, 0)));
        msg->organization()->from7BitString("kde");
        msg->assemble();

        auto item = toItem(msg, 104, 5);
        item.setSize(1002);
        item.setFlags({ Akonadi::MessageFlags::Flagged, Akonadi::MessageFlags::Replied });
        items << item;
    }
    {
        KMime::Message::Ptr msg(new KMime::Message);
        msg->subject()->from7BitString("subject3");

        //Multipart message
        KMime::Content *b = new KMime::Content;
        b->contentType()->setMimeType("text/plain");
        b->setBody("body3");
        msg->addContent(b, true);

        msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
        msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
        msg->date()->setDateTime(QDateTime(QDate(2014, 11, 10), QTime(13, 0, 0)));
        msg->organization()->from7BitString("kde5");
        msg->assemble();

        auto item = toItem(msg, 105, 5);
        item.setSize(1002);
        item.setFlags({ Akonadi::MessageFlags::Flagged, Akonadi::MessageFlags::Replied });
        items << item;
    }
    {
        KMime::Message::Ptr msg(new KMime::Message);
        msg->subject()->from7BitString("subject4");

        //Multipart message
        KMime::Content *b = new KMime::Content;
        b->contentType()->setMimeType("text/plain");
        b->setBody("body4");
        msg->addContent(b, true);

        msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
        msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
        msg->cc()->addAddress("cc@test.com", QStringLiteral("Jane Doe"));
        msg->bcc()->addAddress("bcc@test.com", QStringLiteral("Jane Doe"));
        msg->date()->setDateTime(QDateTime(QDate(2014, 11, 11), QTime(13, 0, 0)));
        msg->replyTo()->from7BitString("test@kde.org");
        KMime::Headers::Generic *header = new KMime::Headers::Generic("Resent-From");
        header->fromUnicodeString(QStringLiteral("resent@kde.org"), "utf-8");
        msg->setHeader(header);
        header = new KMime::Headers::Generic("List-Id");
        header->fromUnicodeString(QStringLiteral("KDE PIM <kde-pim.kde.org>"), "utf-8");
        msg->setHeader(header);
        msg->assemble();

        auto item = toItem(msg, 106, 5);
        item.setSize(1002);
        item.setFlags({ Akonadi::MessageFlags::Flagged, Akonadi::MessageFlags::Replied });
        items << item;
    }
    {
        KMime::Message::Ptr msg(new KMime::Message);
        msg->subject()->from7BitString("all tags");

        //Multipart message
        KMime::Content *b = new KMime::Content;
        b->contentType()->setMimeType("text/plain");
        b->setBody("tags");
        msg->addContent(b, true);

        msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
        msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
        msg->date()->setDateTime(QDateTime(QDate(2014, 11, 11), QTime(13, 0, 0)));
        msg->assemble();

        auto item = toItem(msg, 107, 5);
        item.setSize(1002);
        item.setFlags({ Akonadi::MessageFlags::Seen,
                        Akonadi::MessageFlags::Deleted,
                        Akonadi::MessageFlags::Answered,
                        Akonadi::MessageFlags::Flagged,
                        Akonadi::MessageFlags::HasAttachment,
                        Akonadi::MessageFlags::HasInvitation,
                        Akonadi::MessageFlags::Sent,
                        //Akonadi::MessageFlags::Queued,    //can't have Sent and Queued at the same time
                        Akonadi::MessageFlags::Replied,
                        Akonadi::MessageFlags::Forwarded,
                        Akonadi::MessageFlags::ToAct,
                        Akonadi::MessageFlags::Watched,
                        //Akonadi::MessageFlags::Ignored,   // can't have Watched and Ignored at the same time
                        Akonadi::MessageFlags::Encrypted,
                        /* Akonadi::MessageFlags::Spam,*/
                        Akonadi::MessageFlags::Ham });
        //Spam is exclude from indexer. So we can't add it.
        items << item;
    }
    {
        KMime::Message::Ptr msg(new KMime::Message);
        msg->subject()->from7BitString("Change in qt/qtx11extras[stable]: remove QtWidgets dependency");

        //Multipart message
        KMime::Content *b = new KMime::Content;
        b->contentType()->setMimeType("text/plain");
        b->setBody("body5");
        msg->addContent(b, true);

        msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
        msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
        msg->date()->setDateTime(QDateTime(QDate(2014, 11, 11), QTime(13, 0, 0)));
        msg->assemble();

        auto item = toItem(msg, 108, 5);
        item.setSize(50);
        item.setFlags({ Akonadi::MessageFlags::Flagged, Akonadi::MessageFlags::Replied });
        items << item;
    }

    indexItems(items);
}

void StoreTest::testEmailStore_data()
{
    QTest::addColumn<Akonadi::SearchQuery>("akonadiQuery");
    QTest::addColumn<QVector<Akonadi::Item::Id>>("expectedResults");
    QTest::addColumn<QString>("mimeType");

    Akonadi::SearchTerm allEmailCollections(Akonadi::SearchTerm::RelOr);
    allEmailCollections.addSubTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 3ll));
    allEmailCollections.addSubTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 4ll));
    allEmailCollections.addSubTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 5ll));

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderFrom, QStringLiteral("dvratil@kde.org"),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 3ll));
        QTest::newRow("FROM contains 'dvratil@kde.org'") << aq << QVector<Akonadi::Item::Id>{ 100 }
                                                         << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelAnd);
        Akonadi::SearchTerm t(Akonadi::SearchTerm::RelOr);
        t.addSubTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderFrom, QStringLiteral("Daniel Vrátil"),
                                              Akonadi::SearchTerm::CondContains));
        t.addSubTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, QStringLiteral("Daniel Vrátil"),
                                              Akonadi::SearchTerm::CondContains));
        aq.addTerm(t);
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 3ll));
        QTest::newRow("FROM or CC contains 'Daniel Vrátil'") << aq << QVector<Akonadi::Item::Id>{ 100, 101 }
                                                             << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Seen)));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 3ll));
        QTest::newRow("Seen") << aq << QVector<Akonadi::Item::Id>{ 100, 101 }
                              << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Seen)));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::HasAttachment)));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 3ll));
        QTest::newRow("Seen and HasAttachment") << aq << QVector<Akonadi::Item::Id>{ 101 }
                                                << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("Hospůdka"),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 3ll));
        QTest::newRow("BODY contains 'Hospůdka'") << aq << QVector<Akonadi::Item::Id>{ 102 }
                                                  << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderDate, QDateTime({ 2017, 10, 2 }, { 8, 0 ,0 }),
                                            Akonadi::SearchTerm::CondGreaterThan));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderDate, QDateTime({ 2017, 10, 2 }, { 12, 0, 0 }),
                                            Akonadi::SearchTerm::CondLessThan));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 3ll));
        QTest::newRow("DATE between 8 and 12am") << aq << QVector<Akonadi::Item::Id>{ 101 }
                                                 << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2017, 10, 2)));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 3ll));
        QTest::newRow("DATE on 2017-10-02") << aq << QVector<Akonadi::Item::Id>{ 100, 101 }
                                            << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, 100,
                                            Akonadi::SearchTerm::CondGreaterThan));
        auto term = Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::HasAttachment));
        term.setIsNegated(true);
        aq.addTerm(term);
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 3ll));
        QTest::newRow("SIZE > 100 and not HasAttachment") << aq << QVector<Akonadi::Item::Id>{ 102 }
                                                          << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 4ll));
        QTest::newRow("all emails in collection") << aq << QVector<Akonadi::Item::Id>{ 103 }
                                                  << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1")));
        QTest::newRow("find subject equal") << aq << QVector<Akonadi::Item::Id>{ 103 }
                                            << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        Akonadi::EmailSearchTerm term(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1"));
        term.setIsNegated(true);
        aq.addTerm(term);
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 5ll));
        QTest::newRow("find subject equal negated") << aq << QVector<Akonadi::Item::Id>{ 104, 105, 106, 107, 108 }
                                                    << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject"),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find subject contains") << aq << QVector<Akonadi::Item::Id>{ 103, 104, 105, 106 }
                                               << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("body"),
                                               Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find body contains") << aq << QVector<Akonadi::Item::Id>{ 103, 104, 105, 106, 108 }
                                            << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("mälmöö"),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find utf8 body contains") << aq << QVector<Akonadi::Item::Id>{ 103 }
                                                 << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Headers, QStringLiteral("From:"),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find header contains") << aq << QVector<Akonadi::Item::Id>{ 100, 101, 102, 103, 104, 105, 106, 107, 108 }
                                              << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Message, QStringLiteral("body"),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find message contains") << aq << QVector<Akonadi::Item::Id>{ 103, 104, 105, 106, 108 }
                                               << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelOr);
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1")));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject2")));
        QTest::newRow("or term") << aq << QVector<Akonadi::Item::Id>{ 103, 104 }
                                 << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelAnd);
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1")));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("body1"),
                                            Akonadi::SearchTerm::CondContains));
        QTest::newRow("and term") << aq << QVector<Akonadi::Item::Id>{ 103 }
                                  << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelAnd);
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1")));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("body2")));
        QTest::newRow("and term equal") << aq << QVector<Akonadi::Item::Id>{}
                                        << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject"),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 4ll));
        QTest::newRow("filter by collection") << aq << QVector<Akonadi::Item::Id>{ 103 }
                                              << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Flagged),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by message flag") << aq << QVector<Akonadi::Item::Id>{ 100, 104, 105, 106, 107, 108 }
                                              << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Replied),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by message replied") << aq << QVector<Akonadi::Item::Id>{ 103, 104, 105, 106, 107, 108 }
                                                 << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelAnd);
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Replied),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Encrypted),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by message replied and encrypted") << aq << QVector<Akonadi::Item::Id>{ 103, 107 }
                                                               << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelAnd);
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Seen),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Deleted),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Answered),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Flagged), 
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::HasAttachment),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::HasInvitation),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Sent),
                                            Akonadi::SearchTerm::CondContains));
        //aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Queued), Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Replied),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Forwarded),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::ToAct), 
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Watched),
                                            Akonadi::SearchTerm::CondContains));
        //aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Ignored), Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Encrypted),
                                            Akonadi::SearchTerm::CondContains));
        //Spam is exclude from indexer.
        //aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Spam), Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Ham), 
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by message by all status") << aq << QVector<Akonadi::Item::Id>{ 107 }
                                                       << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1000), Akonadi::SearchTerm::CondGreaterOrEqual));
        QTest::newRow("find by size greater than or equal") << aq << QVector<Akonadi::Item::Id>{ 103, 104, 105, 106, 107 }
                                                            << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1000)));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by size equal") << aq << QVector<Akonadi::Item::Id>{ 103 }
                                            << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1002),
                                            Akonadi::SearchTerm::CondLessOrEqual));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by size less than or equal") << aq << QVector<Akonadi::Item::Id>{ 100, 101, 102, 103, 104, 105, 106, 107, 108 }
                                                         << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1001),
                                            Akonadi::SearchTerm::CondGreaterOrEqual));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by size separate") << aq << QVector<Akonadi::Item::Id>{ 104, 105, 106, 107 }
                                               << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1001), 
                                            Akonadi::SearchTerm::CondGreaterThan));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by size separate (greater than)") << aq << QVector<Akonadi::Item::Id>{ 104, 105, 106, 107 }
                                                              << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderDate, QDateTime(QDate(2013, 11, 10), QTime(12, 30, 0)),
                                            Akonadi::SearchTerm::CondGreaterOrEqual));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by date") << aq << QVector<Akonadi::Item::Id>{ 100, 101, 102, 104, 105, 106, 107, 108 }
                                      << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderDate, QDateTime(QDate(2013, 11, 10), QTime(12, 0, 0))));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by date equal") << aq << QVector<Akonadi::Item::Id>{ 103 }
                                            << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2014, 11, 11)));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by date only (equal condition)") << aq << QVector<Akonadi::Item::Id>{ 106, 107, 108 }
                                                             << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2013, 11, 10),
                                            Akonadi::SearchTerm::CondGreaterOrEqual));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by date only (greater or equal)") << aq << QVector<Akonadi::Item::Id>{ 100, 101, 102, 103, 104, 105, 106, 107, 108 }
                                                              << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2014, 11, 10),
                                            Akonadi::SearchTerm::CondGreaterOrEqual));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by date only greater or equal") << aq << QVector<Akonadi::Item::Id>{ 100, 101, 102, 105, 106, 107, 108 }
                                                            << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2014, 11, 10),
                                            Akonadi::SearchTerm::CondGreaterThan));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by date only greater than") << aq << QVector<Akonadi::Item::Id>{ 100, 101, 102, 106, 107, 108 }
                                                        << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, QStringLiteral("Jane Doe <cc@test.com>")));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by header cc") << aq << QVector<Akonadi::Item::Id>{ 106 }
                                           << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, QStringLiteral("cc@test.com"),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by header cc (contains)") << aq << QVector<Akonadi::Item::Id>{ 106 }
                                                      << KMime::Message::mimeType();
    }

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOrganization, QStringLiteral("kde")));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by header organization (equal)") << aq << QVector<Akonadi::Item::Id>{ 104 }
                                                             << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOrganization, QStringLiteral("kde"),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by header organization (contains)") << aq << QVector<Akonadi::Item::Id>{ 104, 105 }
                                                                << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderBCC, QStringLiteral("Jane Doe <bcc@test.com>"),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by header bcc") << aq << QVector<Akonadi::Item::Id>{ 106 }
                                            << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderReplyTo, QStringLiteral("test@kde.org"),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by reply to") << aq << QVector<Akonadi::Item::Id>{ 106 }
                                          << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, QStringLiteral("kde-pim.kde.org"),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(allEmailCollections);
        QTest::newRow("find by list id") << aq << QVector<Akonadi::Item::Id>{ 106 }
                                         << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelOr);
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Deleted),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, QStringLiteral("kde-pim.kde.org"),
                                            Akonadi::SearchTerm::CondContains));
        QTest::newRow("find by message by deleted status or headerListId") << aq << QVector<Akonadi::Item::Id>{ 106, 107 }
                                                                           << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelOr);
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Deleted),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, QStringLiteral("kde-pim.kde.org"),
                                            Akonadi::SearchTerm::CondContains));

        QVector<qint64> collections;
        QTest::newRow("find by message by deleted status or headerListId in all collections")
            << aq << QVector<Akonadi::Item::Id>{ 106, 107 } << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelAnd);
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Deleted),
                                            Akonadi::SearchTerm::CondContains));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, QStringLiteral("kde-pim.kde.org"),
                                            Akonadi::SearchTerm::CondContains));
        QTest::newRow("find by message by deleted status and headerListId")
            << aq << QVector<Akonadi::Item::Id>{} << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Message, QStringLiteral("subject"), Akonadi::SearchTerm::CondContains));
        QTest::newRow("find by message term") << aq << QVector<Akonadi::Item::Id>{ 100, 101, 102, 103, 104, 105, 106, 107, 108 }
                                              << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, QStringLiteral("CC@TEST.com"),
                                            Akonadi::SearchTerm::CondContains));
        QTest::newRow("find by header cc (contains) with case") << aq << QVector<Akonadi::Item::Id>{ 106 }
                                                                << KMime::Message::mimeType();
    }
#if 0 //Can not work for the moment
    {
        Akonadi::SearchQuery aq;
        //Change in qt/qtx11extras[stable]: remove QtWidgets dependency
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("extras"), Akonadi::SearchTerm::CondContains));
        QSet<qint64> result = QSet<qint64>() << 6;
        QTest::newRow("search extras in subject") << QString::fromLatin1(aq.toJSON()) << allEmailCollections << emailMimeTypes << result;
    }
#endif
    {
        Akonadi::SearchQuery aq;
        //Change in qt/qtx11extras[stable]: remove QtWidgets dependency
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("change"),
                                            Akonadi::SearchTerm::CondContains));
        QTest::newRow("search \"change\" in subject") << aq << QVector<Akonadi::Item::Id>{ 108 }
                                                      << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        //Change in qt/qtx11extras[stable]: remove QtWidgets dependency
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("qtx11extras"),
                                            Akonadi::SearchTerm::CondContains));
        QTest::newRow("search qtx11extras in subject") << aq << QVector<Akonadi::Item::Id>{ 108 }
                                                       << KMime::Message::mimeType();
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderFrom, QStringLiteral("test.com"),
                                            Akonadi::SearchTerm::CondContains));
        QTest::newRow("search by from email part") << aq << QVector<Akonadi::Item::Id>{ 103, 104, 105, 106, 107, 108 }
                                                   << KMime::Message::mimeType();
     }
}

void StoreTest::testEmailStore()
{
    testStore();
}


void StoreTest::indexIncidences()
{
    Akonadi::Item::List items;
    {
        auto event = KCalCore::Event::Ptr::create();
        event->setOrganizer(QStringLiteral("organizer@example.com"));
        event->addAttendee(KCalCore::Attendee::Ptr::create(QStringLiteral("attendee1"), QStringLiteral("attendee1@example.com"),
                                                           false, KCalCore::Attendee::NeedsAction));
        event->addAttendee(KCalCore::Attendee::Ptr::create(QStringLiteral("attendee2"), QStringLiteral("attendee2@example.com"),
                                                           false, KCalCore::Attendee::Accepted));
        event->addAttendee(KCalCore::Attendee::Ptr::create(QStringLiteral("attendee3"), QStringLiteral("attendee3@example.com"),
                                                           false, KCalCore::Attendee::Declined));
        event->addAttendee(KCalCore::Attendee::Ptr::create(QStringLiteral("attendee4"), QStringLiteral("attendee4@example.com"),
                                                           false, KCalCore::Attendee::Tentative));
        event->addAttendee(KCalCore::Attendee::Ptr::create(QStringLiteral("attendee5"), QStringLiteral("attendee5@example.com"),
                                                           false, KCalCore::Attendee::Delegated));
        event->setSummary(QStringLiteral("title"));
        event->setLocation(QStringLiteral("here"));

        items << toItem(event, 200, 6, KCalCore::Event::eventMimeType());
    }

    indexItems(items);
}

void StoreTest::testIncidenceStore_data()
{
    QTest::addColumn<Akonadi::SearchQuery>("akonadiQuery");
    QTest::addColumn<QVector<Akonadi::Item::Id>>("expectedResults");
    QTest::addColumn<QString>("mimeType");

    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Organizer, QStringLiteral("organizer@example.com")));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 6));
        QTest::newRow("find organizer") << aq << QVector<Akonadi::Item::Id>{ 200 }
                                        << QString(KCalCore::Event::eventMimeType());
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Organizer, QStringLiteral("organizer2@example.com")));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 6));
        QTest::newRow("find no organizer") << aq << QVector<Akonadi::Item::Id>{}
                                           << QString(KCalCore::Event::eventMimeType());
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, QStringLiteral("attendee1@example.com0")));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 6));
        QTest::newRow("find events needsAction") << aq << QVector<Akonadi::Item::Id>{ 200 }
                                                 << QString(KCalCore::Event::eventMimeType());
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, QStringLiteral("attendee2@example.com1")));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 6));
        QTest::newRow("find events accepted") << aq << QVector<Akonadi::Item::Id>{ 200 }
                                              << QString(KCalCore::Event::eventMimeType());
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, QStringLiteral("attendee3@example.com2")));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 6));
        QTest::newRow("find events declined") << aq << QVector<Akonadi::Item::Id>{ 200 }
                                              << QString(KCalCore::Event::eventMimeType());
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, QStringLiteral("attendee4@example.com3")));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 6));
        QTest::newRow("find events tentative") << aq << QVector<Akonadi::Item::Id>{ 200 }
                                               << QString(KCalCore::Event::eventMimeType());
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, QStringLiteral("attendee5@example.com4")));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 6));
        QTest::newRow("find events delegated") << aq << QVector<Akonadi::Item::Id>{ 200 }
                                               << QString(KCalCore::Event::eventMimeType());
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, QStringLiteral("attendee5@example.com5")));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 6));
        QTest::newRow("unknown partstatus") << aq << QVector<Akonadi::Item::Id>{}
                                            << QString(KCalCore::Event::eventMimeType());
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Summary, QStringLiteral("title")));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 6));
        QTest::newRow("find event summary") << aq << QVector<Akonadi::Item::Id>{ 200 }
                                            << QString(KCalCore::Event::eventMimeType());
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Location, QStringLiteral("here")));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 6));
        QTest::newRow("find events location") << aq << QVector<Akonadi::Item::Id>{ 200 }
                                              << QString(KCalCore::Event::eventMimeType());
    }
}

void StoreTest::testIncidenceStore()
{
    testStore();
}


void StoreTest::indexNotes()
{
    Akonadi::Item::List items;

    {
        auto msg = KMime::Message::Ptr::create();
        msg->subject()->from7BitString("note");

        //Multipart message
        KMime::Content *b = new KMime::Content;
        b->contentType()->setMimeType("text/plain");
        b->setBody("body note");
        msg->addContent(b, true);
        msg->assemble();

        auto item = toItem(msg, 300, 8, QStringLiteral("text/x-vnd.akonadi.note"));
        item.setSize(1002);
        item.setFlags({ Akonadi::MessageFlags::Flagged, Akonadi::MessageFlags::Replied });
        items << item;
    }
    {
        auto msg = KMime::Message::Ptr::create();
        msg->subject()->from7BitString("note2");

        //Multipart message
        KMime::Content *b = new KMime::Content;
        b->contentType()->setMimeType("text/plain");
        b->setBody("note");
        msg->addContent(b, true);
        msg->assemble();

        auto item = toItem(msg, 301, 8, QStringLiteral("text/x-vnd.akonadi.note"));
        item.setSize(1002);
        item.setFlags({ Akonadi::MessageFlags::Flagged, Akonadi::MessageFlags::Replied });
        items << item;
    }
    {
        auto msg = KMime::Message::Ptr::create();
        msg->subject()->from7BitString("note3");

        //Multipart message
        KMime::Content *b = new KMime::Content;
        b->contentType()->setMimeType("text/plain");
        b->setBody("note3");
        msg->addContent(b, true);
        msg->assemble();

        auto item = toItem(msg, 302, 8, QStringLiteral("text/x-vnd.akonadi.note"));
        item.setSize(1002);
        item.setFlags({ Akonadi::MessageFlags::Flagged, Akonadi::MessageFlags::Replied });
        items << item;
    }

    indexItems(items);
}

void StoreTest::testNoteStore_data()
{
    QTest::addColumn<Akonadi::SearchQuery>("akonadiQuery");
    QTest::addColumn<QVector<Akonadi::Item::Id>>("expectedResults");
    QTest::addColumn<QString>("mimeType");

    const auto notesMimeType = QStringLiteral("text/x-vnd.akonadi.note");
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note")));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 8ll));
        QTest::newRow("find note subject equal") << aq << QVector<Akonadi::Item::Id>{ 300 }
                                                 << notesMimeType;
    }
    {
        Akonadi::SearchQuery aq;
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note1")));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 8ll));
        QTest::newRow("find note subject equal") << aq << QVector<Akonadi::Item::Id>{}
                                                 << notesMimeType;
    }
    {
        Akonadi::SearchQuery aq;
        Akonadi::SearchTerm or(Akonadi::SearchTerm::RelOr);
        or.addSubTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note")));
        or.addSubTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("note")));
        aq.addTerm(or);
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 8ll));
        QTest::newRow("find note subject equal or body equal") << aq << QVector<Akonadi::Item::Id>{ 300, 301 }
                                                               << notesMimeType;
    }
    {
        Akonadi::SearchQuery aq(Akonadi::SearchTerm::RelAnd);
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note3")));
        aq.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("note3")));
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 8ll));
        QTest::newRow("find note subject equal and body equal") << aq << QVector<Akonadi::Item::Id>{ 302 }
                                                                << notesMimeType;
    }
    {
        Akonadi::SearchQuery aq;
        Akonadi::EmailSearchTerm term(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note3"));
        term.setIsNegated(true);
        aq.addTerm(term);
        aq.addTerm(Akonadi::SearchTerm(Akonadi::SearchTerm::Collection, 8ll));
        QTest::newRow("find not subject equal note3") << aq << QVector<Akonadi::Item::Id>{ 300, 301 }
                                                      << notesMimeType;
    }
}

void StoreTest::testNoteStore()
{
    testStore();
}

QTEST_MAIN(StoreTest)
