/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include <Akonadi/Collection>
using namespace Qt::Literals::StringLiterals;

#include <KContacts/Addressee>
#include <KContacts/ContactGroup>
#include <QDir>
#include <QTest>

#include "../agent/calendarindexer.h"
#include "../agent/contactindexer.h"
#include "../agent/emailindexer.h"
#include "../search/calendar/calendarsearchstore.h"
#include "../search/contact/contactsearchstore.h"
#include "../search/email/emailsearchstore.h"
#include "searchplugin.h"
#include <Akonadi/MessageFlags>
#include <Akonadi/SearchQuery>

#include <QElapsedTimer>

Q_DECLARE_METATYPE(QSet<qint64>)
Q_DECLARE_METATYPE(QList<qint64>)

class SearchPluginTest : public QObject
{
    Q_OBJECT
private:
    QString emailDir;
    QString emailContactsDir;
    QString contactsDir;
    QString calendarDir;

    void resultSearch()
    {
        QFETCH(QString, query);
        QFETCH(QList<qint64>, collections);
        QFETCH(QStringList, mimeTypes);
        QFETCH(QSet<qint64>, expectedResult);

        qDebug() << "starting search";
        QElapsedTimer t;
        t.start();
        SearchPlugin plugin;
        const QSet<qint64> result = plugin.search(query, collections, mimeTypes);
        qDebug() << "result:" << result << "(in" << t.elapsed() << "ms)";
        QEXPECT_FAIL("contact by name (oe)", "Does not work for the moment", Continue);
        QEXPECT_FAIL("search extras in subject", "Does not work for the moment", Continue);
        QCOMPARE(result, expectedResult);
    }

private Q_SLOTS:
    void initTestCase()
    {
        emailDir = QDir::tempPath() + "/searchplugintest/email/"_L1;
        emailContactsDir = QDir::tempPath() + "/searchplugintest/emailcontacts/"_L1;
        contactsDir = QDir::tempPath() + "/searchplugintest/contacts/"_L1;
        calendarDir = QDir::tempPath() + "/searchplugintest/calendar/"_L1;

        QDir dir;
        QVERIFY(QDir(QDir::tempPath() + u"/searchplugintest"_s).removeRecursively());
        QVERIFY(dir.mkpath(emailDir));
        QVERIFY(dir.mkpath(emailContactsDir));
        QVERIFY(dir.mkpath(contactsDir));
        QVERIFY(dir.mkpath(calendarDir));

        qDebug() << "indexing sample data";
        qDebug() << emailDir;
        qDebug() << emailContactsDir;
        qDebug() << calendarDir;

        EmailIndexer emailIndexer(emailDir, emailContactsDir);
        ContactIndexer contactIndexer(contactsDir);
        CalendarIndexer calendarIndexer(calendarDir);

        {
            auto msg = std::make_shared<KMime::Message>();
            msg->subject()->from7BitString("subject1");
            msg->contentType()->setMimeType("text/plain");
            msg->contentType()->setCharset("utf-8");
            msg->setBody("body1 mälmöö");
            msg->from()->addAddress("john@test.com", u"John Doe"_s);
            msg->to()->addAddress("jane@test.com", u"Jane Doe"_s);
            msg->date()->setDateTime(QDateTime(QDate(2013, 11, 10), QTime(12, 0, 0)));
            msg->assemble();

            Akonadi::Item item(u"message/rfc822"_s);
            item.setId(1);
            item.setSize(1000);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(1));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Replied << Akonadi::MessageFlags::Encrypted);
            emailIndexer.index(item);
        }
        {
            auto msg = std::make_shared<KMime::Message>();
            msg->contentType()->setMimeType("multipart/mixed");
            msg->subject()->from7BitString("subject2");

            // Multipart message
            auto b = std::unique_ptr<KMime::Content>(new KMime::Content);
            b->contentType()->setMimeType("text/plain");
            b->setBody("body2");
            msg->prependContent(std::move(b));

            msg->from()->addAddress("john@test.com", u"John Doe"_s);
            msg->to()->addAddress("jane@test.com", u"Jane Doe"_s);
            msg->date()->setDateTime(QDateTime(QDate(2013, 11, 10), QTime(13, 0, 0)));
            msg->organization()->from7BitString("kde");
            msg->assemble();

            Akonadi::Item item(u"message/rfc822"_s);
            item.setId(2);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            emailIndexer.index(item);
        }
        {
            auto msg = std::make_shared<KMime::Message>();
            msg->contentType()->setMimeType("multipart/mixed");
            msg->subject()->from7BitString("subject3");

            // Multipart message
            auto b = std::unique_ptr<KMime::Content>(new KMime::Content);
            b->contentType()->setMimeType("text/plain");
            b->setBody("body3");
            msg->prependContent(std::move(b));

            msg->from()->addAddress("john@test.com", u"John Doe"_s);
            msg->to()->addAddress("jane@test.com", u"Jane Doe"_s);
            msg->date()->setDateTime(QDateTime(QDate(2014, 11, 10), QTime(13, 0, 0)));
            msg->organization()->from7BitString("kde5");
            msg->assemble();

            Akonadi::Item item(u"message/rfc822"_s);
            item.setId(3);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            emailIndexer.index(item);
        }
        {
            auto msg = std::make_shared<KMime::Message>();
            msg->contentType()->setMimeType("multipart/mixed");
            msg->subject()->from7BitString("subject4");

            // Multipart message
            auto b = std::unique_ptr<KMime::Content>(new KMime::Content);
            b->contentType()->setMimeType("text/plain");
            b->setBody("body4");
            msg->prependContent(std::move(b));

            msg->from()->addAddress("john_blue@test.com", u"John Doe"_s);
            msg->to()->addAddress("jane@test.com", u"Jane Doe"_s);
            msg->cc()->addAddress("cc@test.com", u"Jane Doe"_s);
            msg->bcc()->addAddress("bcc@test.com", u"Jane Doe"_s);
            msg->date()->setDateTime(QDateTime(QDate(2014, 11, 11), QTime(13, 0, 0)));
            msg->replyTo()->from7BitString("test@kde.org");
            auto header = std::unique_ptr<KMime::Headers::Generic>(new KMime::Headers::Generic("Resent-From"));
            header->fromUnicodeString(u"resent@kde.org"_s);
            msg->setHeader(std::move(header));
            header = std::unique_ptr<KMime::Headers::Generic>(new KMime::Headers::Generic("List-Id"));
            header->fromUnicodeString(u"KDE PIM <kde-pim.kde.org>"_s);
            msg->setHeader(std::move(header));

            msg->assemble();

            Akonadi::Item item(u"message/rfc822"_s);
            item.setId(4);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            emailIndexer.index(item);
        }
        {
            auto msg = std::make_shared<KMime::Message>();
            msg->contentType()->setMimeType("multipart/mixed");
            msg->subject()->from7BitString("all tags");

            // Multipart message
            auto b = std::unique_ptr<KMime::Content>(new KMime::Content);
            b->contentType()->setMimeType("text/plain");
            b->setBody("tags");
            msg->prependContent(std::move(b));

            msg->from()->addAddress("john@test.com", u"John Doe"_s);
            msg->to()->addAddress("jane@test.com", u"Jane Doe"_s);
            msg->date()->setDateTime(QDateTime(QDate(2014, 11, 11), QTime(13, 0, 0)));
            msg->assemble();

            Akonadi::Item item(u"message/rfc822"_s);
            item.setId(5);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Seen << Akonadi::MessageFlags::Deleted << Akonadi::MessageFlags::Answered
                                                 << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::HasAttachment
                                                 << Akonadi::MessageFlags::HasInvitation
                                                 << Akonadi::MessageFlags::Sent
                                                 //<< Akonadi::MessageFlags::Queued    //can't have Sent and Queued at the same time
                                                 << Akonadi::MessageFlags::Replied << Akonadi::MessageFlags::Forwarded << Akonadi::MessageFlags::ToAct
                                                 << Akonadi::MessageFlags::Watched
                                                 //<< Akonadi::MessageFlags::Ignored   // can't have Watched and Ignored at the same time
                                                 << Akonadi::MessageFlags::Encrypted
                                                 /*<< Akonadi::MessageFlags::Spam*/
                                                 << Akonadi::MessageFlags::Ham);
            // Spam is exclude from indexer. So we can't add it.
            emailIndexer.index(item);
        }
        {
            auto msg = std::make_shared<KMime::Message>();
            msg->subject()->from7BitString("Change in qt/qtx11extras[stable]: remove QtWidgets dependency");
            msg->contentType()->setMimeType("multipart/mixed");

            // Multipart message
            auto b = std::unique_ptr<KMime::Content>(new KMime::Content);
            b->contentType()->setMimeType("text/plain");
            b->setBody("body5");
            msg->prependContent(std::move(b));

            msg->from()->addAddress("john@test.com", u"John Doe"_s);
            msg->to()->addAddress("jane@test.com", u"Jane Doe"_s);
            msg->date()->setDateTime(QDateTime(QDate(2014, 11, 11), QTime(13, 0, 0)));
            msg->assemble();

            Akonadi::Item item(u"message/rfc822"_s);
            item.setId(6);
            item.setSize(50);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            emailIndexer.index(item);
        }
        emailIndexer.commit();

        // Contact item
        {
            KContacts::Addressee addressee;
            addressee.setUid(u"uid1"_s);
            addressee.setName(u"John Doe"_s);
            addressee.setFormattedName(u"John Doe"_s);
            addressee.setNickName(u"JD"_s);
            addressee.setEmails({u"john@test.com"_s});
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01).startOfDay()));
            Akonadi::Item item(KContacts::Addressee::mimeType());
            item.setId(100);
            item.setPayload(addressee);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }
        {
            KContacts::Addressee addressee;
            addressee.setUid(u"uid2"_s);
            addressee.setName(u"Jane Doe"_s);
            addressee.setEmails({u"jane@test.com"_s, u"jack_sparrow@test.com"_s});
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01).startOfDay()));
            Akonadi::Item item(KContacts::Addressee::mimeType());
            item.setId(101);
            item.setPayload(addressee);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }
        {
            KContacts::Addressee addressee;
            addressee.setUid(u"uid2"_s);
            addressee.setName(u"Jane Doe"_s);
            addressee.setEmails({u"JANE@TEST.COM"_s});
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01).startOfDay()));
            Akonadi::Item item(KContacts::Addressee::mimeType());
            item.setId(102);
            item.setPayload(addressee);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }
        {
            KContacts::Addressee addressee;
            addressee.setUid(u"abcd-efgh-1234-5678"_s);
            addressee.setName(u"Dan Vrátil"_s);
            addressee.setEmails({u"dan@test.com"_s});
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01).startOfDay()));
            Akonadi::Item item(KContacts::Addressee::mimeType());
            item.setId(105);
            item.setPayload(addressee);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }

        {
            KContacts::ContactGroup group;
            group.setName(u"group1"_s);
            Akonadi::Item item(KContacts::ContactGroup::mimeType());
            item.setId(103);
            item.setPayload(group);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }
        {
            KContacts::ContactGroup group;
            group.setName(u"group3"_s);
            Akonadi::Item item(KContacts::ContactGroup::mimeType());
            item.setId(104);
            item.setPayload(group);
            item.setParentCollection(Akonadi::Collection(4));
            contactIndexer.index(item);
        }
        contactIndexer.commit();

        // Calendar item
        {
            KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
            KCalendarCore::Attendee attendee(u"attendee1"_s, u"attendee1@example.com"_s, false, KCalendarCore::Attendee::NeedsAction);
            event->setOrganizer(u"organizer@example.com"_s);
            event->addAttendee(attendee);
            attendee = KCalendarCore::Attendee(u"attendee2"_s, u"attendee2@example.com"_s, false, KCalendarCore::Attendee::Accepted);
            event->addAttendee(attendee);
            attendee = KCalendarCore::Attendee(u"attendee3"_s, u"attendee3@example.com"_s, false, KCalendarCore::Attendee::Declined);
            event->addAttendee(attendee);
            attendee = KCalendarCore::Attendee(u"attendee4"_s, u"attendee4@example.com"_s, false, KCalendarCore::Attendee::Tentative);
            event->addAttendee(attendee);
            attendee = KCalendarCore::Attendee(u"attendee5"_s, u"attendee5@example.com"_s, false, KCalendarCore::Attendee::Delegated);
            event->addAttendee(attendee);

            event->setSummary(u"title"_s);
            event->setLocation(u"here"_s);

            Akonadi::Item item(KCalendarCore::Event::eventMimeType());
            item.setId(2001);
            item.setPayload<KCalendarCore::Event::Ptr>(event);
            item.setParentCollection(Akonadi::Collection(6));
            calendarIndexer.index(item);
        }
        calendarIndexer.commit();

        auto emailSearchStore = new Akonadi::Search::EmailSearchStore();
        emailSearchStore->setDbPath(emailDir);
        auto contactSearchStore = new Akonadi::Search::ContactSearchStore();
        contactSearchStore->setDbPath(contactsDir);
        auto calendarSearchStore = new Akonadi::Search::CalendarSearchStore();
        calendarSearchStore->setDbPath(calendarDir);

        Akonadi::Search::SearchStore::overrideSearchStores(QList<Akonadi::Search::SearchStore *>()
                                                           << emailSearchStore << contactSearchStore << calendarSearchStore);
    }

    void cleanupTestCase()
    {
        QVERIFY(QDir(QDir::tempPath() + u"/searchplugintest"_s).removeRecursively());
    }

    void testCalendarSearch_data()
    {
        QTest::addColumn<QString>("query");
        QTest::addColumn<QList<qint64>>("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64>>("expectedResult");
        const QStringList calendarMimeTypes({KCalendarCore::Event::eventMimeType()});
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Organizer, u"organizer@example.com"_s, Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find organizer") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Organizer, u"organizer2@example.com"_s, Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections({6});
            QSet<qint64> result;
            QTest::newRow("find no organizer") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, u"attendee1@example.com0"_s, Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find events needsAction") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, u"attendee2@example.com1"_s, Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find events accepted") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, u"attendee3@example.com2"_s, Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find events declined") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, u"attendee4@example.com3"_s, Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find events tentative") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, u"attendee5@example.com4"_s, Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find events delegated") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, u"attendee5@example.com5"_s, Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections({6});
            QSet<qint64> result;
            QTest::newRow("unknown partstatus") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Summary, u"title"_s, Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find event summary") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Location, u"here"_s, Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find events location") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
    }

    void testCalendarSearch()
    {
        resultSearch();
    }

    void testContactSearch()
    {
        resultSearch();
    }

    void testContactSearch_data()
    {
        QTest::addColumn<QString>("query");
        QTest::addColumn<QList<qint64>>("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64>>("expectedResult");
        const QStringList contactMimeTypes({KContacts::Addressee::mimeType()});
        const QStringList contactGroupMimeTypes({KContacts::ContactGroup::mimeType()});
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, u"John"_s, Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result({100});
            QTest::newRow("contact by name") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, u"John"_s, Akonadi::SearchTerm::CondContains));

            QList<qint64> collections({4});
            QSet<qint64> result;
            QTest::newRow("contact collectionfilter") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, u"john"_s, Akonadi::SearchTerm::CondContains));

            QList<qint64> collections({3});
            QSet<qint64> result({100});
            QTest::newRow("contact by lowercase name") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Nickname, u"JD"_s, Akonadi::SearchTerm::CondContains));

            QList<qint64> collections({3});
            QSet<qint64> result({100});
            QTest::newRow("contact by nickname") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Uid, u"uid1"_s, Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections({3});
            QSet<qint64> result({100});
            QTest::newRow("contact by uid") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Uid, u"abcd-efgh-1234-5678"_s, Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections({3});
            QSet<qint64> result({105});
            QTest::newRow("contact by uid 2") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, u"JANE@TEST.COM"_s, Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections({3});
            QSet<qint64> result({101, 102});
            QTest::newRow("contact by email (JANE@TEST.COM)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, u"jack_sparrow@test.com"_s, Akonadi::SearchTerm::CondContains));
            QList<qint64> collections = {3};
            QSet<qint64> result = {101};
            QTest::newRow("contact by email (jack_sparrow@test.com)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, u"jack_sparrow"_s, Akonadi::SearchTerm::CondContains));
            QList<qint64> collections = {3};
            QSet<qint64> result = {101};
            QTest::newRow("contact by email (jack_sparrow)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, u"jack"_s, Akonadi::SearchTerm::CondContains));
            QList<qint64> collections = {3};
            QSet<qint64> result = {101};
            QTest::newRow("contact by email (jack)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, u"sparrow"_s, Akonadi::SearchTerm::CondContains));
            QList<qint64> collections = {3};
            QSet<qint64> result = {101};
            QTest::newRow("contact by email (sparrow)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, u"Doe"_s, Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result({100, 101, 102});
            QTest::newRow("contact by name (Doe)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, u"Do"_s, Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result({100, 101, 102});
            QTest::newRow("contact by name (Do)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, u"group1"_s, Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result({103});
            QTest::newRow("contact group by name (group1)") << QString::fromLatin1(query.toJSON()) << collections << contactGroupMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, u"group2"_s, Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result;
            QTest::newRow("contact group by name (group2)") << QString::fromLatin1(query.toJSON()) << collections << contactGroupMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, u"group3"_s, Akonadi::SearchTerm::CondContains));

            QList<qint64> collections({4});
            QSet<qint64> result({104});
            QTest::newRow("contact group by name (group3 in collection 4)")
                << QString::fromLatin1(query.toJSON()) << collections << contactGroupMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, u"group3"_s, Akonadi::SearchTerm::CondContains));

            QList<qint64> collections({3});
            QSet<qint64> result;
            QTest::newRow("contact group by name (group3 in collection 3)")
                << QString::fromLatin1(query.toJSON()) << collections << contactGroupMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, u"oe"_s, Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result({100, 101, 102});
            QTest::newRow("contact by name (oe)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
    }

    void testEmailSearch_data()
    {
        QTest::addColumn<QString>("query");
        QTest::addColumn<QList<qint64>>("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64>>("expectedResult");
        const QStringList emailMimeTypes = QStringList() << u"message/rfc822"_s;
        const QList<qint64> allEmailCollections({1, 2});
        {
            Akonadi::SearchQuery query;
            QList<qint64> collections({1});
            QSet<qint64> result = {1};
            QTest::newRow("all emails in collection") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, u"subject1"_s, Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections({1});
            QSet<qint64> result({1});
            QTest::newRow("find subject equal") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            Akonadi::EmailSearchTerm term(Akonadi::EmailSearchTerm::Subject, u"subject1"_s, Akonadi::SearchTerm::CondEqual);
            term.setIsNegated(true);
            query.addTerm(term);
            QList<qint64> collections({2});
            QSet<qint64> result({2, 3, 4, 5, 6});
            QTest::newRow("find subject equal negated") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, u"subject"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1, 2, 3, 4});
            QTest::newRow("find subject contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, u"body"_s, Akonadi::SearchTerm::CondContains));
            QList<qint64> collections({1, 2, 3, 4});
            QSet<qint64> result({1, 2, 3, 4, 6});
            QTest::newRow("find body contains") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, u"mälmöö"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1});
            QTest::newRow("find utf8 body contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Headers, u"From:"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1, 2, 3, 4, 5, 6});
            QTest::newRow("find header contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Message, u"body"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1, 2, 3, 4, 6});
            QTest::newRow("find message contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, u"subject1"_s, Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, u"subject2"_s, Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result({1, 2});
            QTest::newRow("or term") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, u"subject1"_s, Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, u"body1"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1});
            QTest::newRow("and term") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, u"subject1"_s, Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, u"body2"_s, Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result;
            QTest::newRow("and term equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, u"subject"_s, Akonadi::SearchTerm::CondContains));
            QList<qint64> collections({1});
            QSet<qint64> result({1});
            QTest::newRow("filter by collection") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Flagged),
                                                   Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({2, 3, 4, 5, 6});
            QTest::newRow("find by message flag") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Replied),
                                                   Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1, 2, 3, 4, 5, 6});
            QTest::newRow("find by message replied") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Replied),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Encrypted),
                                                   Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1, 5});
            QTest::newRow("find by message replied and encrypted") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Seen),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Deleted),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Answered),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Flagged),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::HasAttachment),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::HasInvitation),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Sent),
                                                   Akonadi::SearchTerm::CondContains));
            // query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Queued),
            // Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Replied),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Forwarded),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::ToAct),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Watched),
                                                   Akonadi::SearchTerm::CondContains));
            // query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Ignored),
            // Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Encrypted),
                                                   Akonadi::SearchTerm::CondContains));
            // Spam is exclude from indexer.
            // query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Spam),
            // Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Ham),
                                                   Akonadi::SearchTerm::CondContains));

            QSet<qint64> result({5});
            QTest::newRow("find by message by all status") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1000), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result({1, 2, 3, 4, 5, 6});
            QTest::newRow("find by size greater than equal great or equal")
                << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1000), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result({1});
            QTest::newRow("find by size equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1002), Akonadi::SearchTerm::CondLessOrEqual));
            QSet<qint64> result({1, 2, 3, 4, 5});
            QTest::newRow("find by size greater than equal less or equal")
                << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1001), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result({2, 3, 4, 5, 6});
            QTest::newRow("find by size separate") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1001), Akonadi::SearchTerm::CondGreaterThan));
            QSet<qint64> result({2, 3, 4, 5, 6});
            QTest::newRow("find by size separate (greater than)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderDate,
                                                   QDateTime(QDate(2013, 11, 10), QTime(12, 30, 0)),
                                                   Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result({2, 3, 4, 5, 6});
            QTest::newRow("find by date") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderDate,
                                                   QDateTime(QDate(2013, 11, 10), QTime(12, 0, 0)),
                                                   Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result({1});
            QTest::newRow("find by date equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2014, 11, 11), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result({4, 5, 6});
            QTest::newRow("find by date only (equal condition)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2013, 11, 10), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result({1, 2, 3, 4, 5, 6});
            QTest::newRow("find by date only (greater or equal)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2014, 11, 10), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result({3, 4, 5, 6});
            QTest::newRow("find by date only greater or equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2014, 11, 10), Akonadi::SearchTerm::CondGreaterThan));
            QSet<qint64> result({4, 5, 6});
            QTest::newRow("find by date only greater than") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, u"Jane Doe <cc@test.com>"_s, Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result({4});
            QTest::newRow("find by header cc") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, u"cc@test.com"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({4});
            QTest::newRow("find by header cc (contains)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOrganization, u"kde"_s, Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result({2});
            QTest::newRow("find by header organization (equal)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOrganization, u"kde"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({2, 3});
            QTest::newRow("find by header organization (contains)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderBCC, u"Jane Doe <bcc@test.com>"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({4});
            QTest::newRow("find by header bcc") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderReplyTo, u"test@kde.org"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({4});
            QTest::newRow("find by reply to") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, u"kde-pim.kde.org"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({4});
            QTest::newRow("find by list id") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Deleted),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, u"kde-pim.kde.org"_s, Akonadi::SearchTerm::CondContains));

            QSet<qint64> result({4, 5});
            QTest::newRow("find by message by deleted status or headerListId")
                << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Deleted),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, u"kde-pim.kde.org"_s, Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result({4, 5});
            QTest::newRow("find by message by deleted status or headerListId in all collections")
                << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Deleted),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, u"kde-pim.kde.org"_s, Akonadi::SearchTerm::CondContains));

            QSet<qint64> result;
            QTest::newRow("find by message by deleted status and headerListId")
                << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Message, u"subject"_s, Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result({1, 2, 3, 4, 5, 6});
            QTest::newRow("find by message term") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, u"CC@TEST.com"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({4});
            QTest::newRow("find by header cc (contains) with case") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            // Change in qt/qtx11extras[stable]: remove QtWidgets dependency
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, u"extras"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({6});
            QTest::newRow("search extras in subject") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            // Change in qt/qtx11extras[stable]: remove QtWidgets dependency
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, u"change"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({6});
            QTest::newRow("search \"change\" in subject") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            // Change in qt/qtx11extras[stable]: remove QtWidgets dependency
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, u"qtx11extras"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({6});
            QTest::newRow("search qtx11extras in subject") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderFrom, u"test.com"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1, 2, 3, 4, 5, 6});
            QTest::newRow("search by from email part") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderFrom, u"john_blue@test.com"_s, Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 4;
            QTest::newRow("search by from email part") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
    }

    void testEmailSearch()
    {
        resultSearch();
    }
};

QTEST_GUILESS_MAIN(SearchPluginTest)

#include "searchplugintest.moc"
