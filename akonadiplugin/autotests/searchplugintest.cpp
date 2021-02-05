/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include <AkonadiCore/Collection>
#include <KContacts/Addressee>
#include <KContacts/ContactGroup>
#include <QDir>
#include <QTest>

#include "searchplugin.h"
#include <../agent/akonotesindexer.h>
#include <../agent/calendarindexer.h>
#include <../agent/contactindexer.h>
#include <../agent/emailindexer.h>
#include <../search/calendar/calendarsearchstore.h>
#include <../search/contact/contactsearchstore.h>
#include <../search/email/emailsearchstore.h>
#include <../search/note/notesearchstore.h>
#include <Akonadi/KMime/MessageFlags>
#include <AkonadiCore/searchquery.h>

#include <QElapsedTimer>

Q_DECLARE_METATYPE(QSet<qint64>)
Q_DECLARE_METATYPE(QVector<qint64>)

class SearchPluginTest : public QObject
{
    Q_OBJECT
private:
    QString emailDir;
    QString emailContactsDir;
    QString contactsDir;
    QString noteDir;
    QString calendarDir;

    void resultSearch()
    {
        QFETCH(QString, query);
        QFETCH(QVector<qint64>, collections);
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
        emailDir = QDir::tempPath() + QLatin1String("/searchplugintest/email/");
        emailContactsDir = QDir::tempPath() + QLatin1String("/searchplugintest/emailcontacts/");
        contactsDir = QDir::tempPath() + QLatin1String("/searchplugintest/contacts/");
        noteDir = QDir::tempPath() + QLatin1String("/searchplugintest/notes/");
        calendarDir = QDir::tempPath() + QLatin1String("/searchplugintest/calendar/");

        QDir dir;
        QVERIFY(QDir(QDir::tempPath() + QStringLiteral("/searchplugintest")).removeRecursively());
        QVERIFY(dir.mkpath(emailDir));
        QVERIFY(dir.mkpath(emailContactsDir));
        QVERIFY(dir.mkpath(contactsDir));
        QVERIFY(dir.mkpath(noteDir));
        QVERIFY(dir.mkpath(calendarDir));

        qDebug() << "indexing sample data";
        qDebug() << emailDir;
        qDebug() << emailContactsDir;
        qDebug() << noteDir;
        qDebug() << calendarDir;

        EmailIndexer emailIndexer(emailDir, emailContactsDir);
        ContactIndexer contactIndexer(contactsDir);
        AkonotesIndexer noteIndexer(noteDir);
        CalendarIndexer calendarIndexer(calendarDir);

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

            Akonadi::Item item(QStringLiteral("message/rfc822"));
            item.setId(1);
            item.setSize(1000);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(1));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Replied << Akonadi::MessageFlags::Encrypted);
            emailIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("subject2");

            // Multipart message
            auto b = new KMime::Content;
            b->contentType()->setMimeType("text/plain");
            b->setBody("body2");
            msg->addContent(b, true);

            msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
            msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
            msg->date()->setDateTime(QDateTime(QDate(2013, 11, 10), QTime(13, 0, 0)));
            msg->organization()->from7BitString("kde");
            msg->assemble();

            Akonadi::Item item(QStringLiteral("message/rfc822"));
            item.setId(2);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            emailIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("subject3");

            // Multipart message
            auto b = new KMime::Content;
            b->contentType()->setMimeType("text/plain");
            b->setBody("body3");
            msg->addContent(b, true);

            msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
            msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
            msg->date()->setDateTime(QDateTime(QDate(2014, 11, 10), QTime(13, 0, 0)));
            msg->organization()->from7BitString("kde5");
            msg->assemble();

            Akonadi::Item item(QStringLiteral("message/rfc822"));
            item.setId(3);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            emailIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("subject4");

            // Multipart message
            auto b = new KMime::Content;
            b->contentType()->setMimeType("text/plain");
            b->setBody("body4");
            msg->addContent(b, true);

            msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
            msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
            msg->cc()->addAddress("cc@test.com", QStringLiteral("Jane Doe"));
            msg->bcc()->addAddress("bcc@test.com", QStringLiteral("Jane Doe"));
            msg->date()->setDateTime(QDateTime(QDate(2014, 11, 11), QTime(13, 0, 0)));
            msg->replyTo()->from7BitString("test@kde.org");
            auto header = new KMime::Headers::Generic("Resent-From");
            header->fromUnicodeString(QStringLiteral("resent@kde.org"), "utf-8");
            msg->setHeader(header);
            header = new KMime::Headers::Generic("List-Id");
            header->fromUnicodeString(QStringLiteral("KDE PIM <kde-pim.kde.org>"), "utf-8");
            msg->setHeader(header);

            msg->assemble();

            Akonadi::Item item(QStringLiteral("message/rfc822"));
            item.setId(4);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            emailIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("all tags");

            // Multipart message
            auto b = new KMime::Content;
            b->contentType()->setMimeType("text/plain");
            b->setBody("tags");
            msg->addContent(b, true);

            msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
            msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
            msg->date()->setDateTime(QDateTime(QDate(2014, 11, 11), QTime(13, 0, 0)));
            msg->assemble();

            Akonadi::Item item(QStringLiteral("message/rfc822"));
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
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("Change in qt/qtx11extras[stable]: remove QtWidgets dependency");

            // Multipart message
            auto b = new KMime::Content;
            b->contentType()->setMimeType("text/plain");
            b->setBody("body5");
            msg->addContent(b, true);

            msg->from()->addAddress("john@test.com", QStringLiteral("John Doe"));
            msg->to()->addAddress("jane@test.com", QStringLiteral("Jane Doe"));
            msg->date()->setDateTime(QDateTime(QDate(2014, 11, 11), QTime(13, 0, 0)));
            msg->assemble();

            Akonadi::Item item(QStringLiteral("message/rfc822"));
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
            addressee.setUid(QStringLiteral("uid1"));
            addressee.setName(QStringLiteral("John Doe"));
            addressee.setFormattedName(QStringLiteral("John Doe"));
            addressee.setNickName(QStringLiteral("JD"));
            addressee.setEmails({QStringLiteral("john@test.com")});
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01)));
#else
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01).startOfDay()));
#endif
            Akonadi::Item item(KContacts::Addressee::mimeType());
            item.setId(100);
            item.setPayload(addressee);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }
        {
            KContacts::Addressee addressee;
            addressee.setUid(QStringLiteral("uid2"));
            addressee.setName(QStringLiteral("Jane Doe"));
            addressee.setEmails({QStringLiteral("jane@test.com")});
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01)));
#else
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01).startOfDay()));
#endif
            Akonadi::Item item(KContacts::Addressee::mimeType());
            item.setId(101);
            item.setPayload(addressee);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }
        {
            KContacts::Addressee addressee;
            addressee.setUid(QStringLiteral("uid2"));
            addressee.setName(QStringLiteral("Jane Doe"));
            addressee.setEmails({QStringLiteral("JANE@TEST.COM")});
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01)));
#else
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01).startOfDay()));
#endif
            Akonadi::Item item(KContacts::Addressee::mimeType());
            item.setId(102);
            item.setPayload(addressee);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }
        {
            KContacts::Addressee addressee;
            addressee.setUid(QStringLiteral("abcd-efgh-1234-5678"));
            addressee.setName(QStringLiteral("Dan Vrátil"));
            addressee.setEmails({QStringLiteral("dan@test.com")});
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01)));
#else
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01).startOfDay()));
#endif
            Akonadi::Item item(KContacts::Addressee::mimeType());
            item.setId(105);
            item.setPayload(addressee);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }
        {
            KContacts::ContactGroup group;
            group.setName(QStringLiteral("group1"));
            Akonadi::Item item(KContacts::ContactGroup::mimeType());
            item.setId(103);
            item.setPayload(group);
            item.setParentCollection(Akonadi::Collection(3));
            contactIndexer.index(item);
        }
        {
            KContacts::ContactGroup group;
            group.setName(QStringLiteral("group3"));
            Akonadi::Item item(KContacts::ContactGroup::mimeType());
            item.setId(104);
            item.setPayload(group);
            item.setParentCollection(Akonadi::Collection(4));
            contactIndexer.index(item);
        }
        contactIndexer.commit();

        // Note item
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("note");

            // Multipart message
            auto b = new KMime::Content;
            b->contentType()->setMimeType("text/plain");
            b->setBody("body note");
            msg->addContent(b, true);
            msg->assemble();

            Akonadi::Item item(QStringLiteral("text/x-vnd.akonadi.note"));
            item.setId(1000);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(5));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            noteIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("note2");

            // Multipart message
            auto b = new KMime::Content;
            b->contentType()->setMimeType("text/plain");
            b->setBody("note");
            msg->addContent(b, true);
            msg->assemble();

            Akonadi::Item item(QStringLiteral("text/x-vnd.akonadi.note"));
            item.setId(1001);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(5));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            noteIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("note3");

            // Multipart message
            auto b = new KMime::Content;
            b->contentType()->setMimeType("text/plain");
            b->setBody("note3");
            msg->addContent(b, true);
            msg->assemble();

            Akonadi::Item item(QStringLiteral("text/x-vnd.akonadi.note"));
            item.setId(1002);
            item.setSize(1002);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(5));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            noteIndexer.index(item);
        }

        // Calendar item
        {
            KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
            KCalendarCore::Attendee attendee(QStringLiteral("attendee1"), QStringLiteral("attendee1@example.com"), false, KCalendarCore::Attendee::NeedsAction);
            event->setOrganizer(QStringLiteral("organizer@example.com"));
            event->addAttendee(attendee);
            attendee = KCalendarCore::Attendee(QStringLiteral("attendee2"), QStringLiteral("attendee2@example.com"), false, KCalendarCore::Attendee::Accepted);
            event->addAttendee(attendee);
            attendee = KCalendarCore::Attendee(QStringLiteral("attendee3"), QStringLiteral("attendee3@example.com"), false, KCalendarCore::Attendee::Declined);
            event->addAttendee(attendee);
            attendee = KCalendarCore::Attendee(QStringLiteral("attendee4"), QStringLiteral("attendee4@example.com"), false, KCalendarCore::Attendee::Tentative);
            event->addAttendee(attendee);
            attendee = KCalendarCore::Attendee(QStringLiteral("attendee5"), QStringLiteral("attendee5@example.com"), false, KCalendarCore::Attendee::Delegated);
            event->addAttendee(attendee);

            event->setSummary(QStringLiteral("title"));
            event->setLocation(QStringLiteral("here"));

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
        auto noteSearchStore = new Akonadi::Search::NoteSearchStore();
        noteSearchStore->setDbPath(noteDir);
        auto calendarSearchStore = new Akonadi::Search::CalendarSearchStore();
        calendarSearchStore->setDbPath(calendarDir);

        Akonadi::Search::SearchStore::overrideSearchStores(QList<Akonadi::Search::SearchStore *>()
                                                           << emailSearchStore << contactSearchStore << noteSearchStore << calendarSearchStore);
    }

    void cleanupTestCase()
    {
        QVERIFY(QDir(QDir::tempPath() + QStringLiteral("/searchplugintest")).removeRecursively());
    }

    void testCalendarSearch_data()
    {
        QTest::addColumn<QString>("query");
        QTest::addColumn<QVector<qint64>>("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64>>("expectedResult");
        const QStringList calendarMimeTypes({KCalendarCore::Event::eventMimeType()});
        {
            Akonadi::SearchQuery query;
            query.addTerm(
                Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Organizer, QStringLiteral("organizer@example.com"), Akonadi::SearchTerm::CondEqual));

            QVector<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find organizer") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Organizer,
                                                       QStringLiteral("organizer2@example.com"),
                                                       Akonadi::SearchTerm::CondEqual));

            QVector<qint64> collections({6});
            QSet<qint64> result;
            QTest::newRow("find no organizer") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus,
                                                       QStringLiteral("attendee1@example.com0"),
                                                       Akonadi::SearchTerm::CondEqual));
            QVector<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find events needsAction") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus,
                                                       QStringLiteral("attendee2@example.com1"),
                                                       Akonadi::SearchTerm::CondEqual));
            QVector<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find events accepted") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus,
                                                       QStringLiteral("attendee3@example.com2"),
                                                       Akonadi::SearchTerm::CondEqual));
            QVector<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find events declined") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus,
                                                       QStringLiteral("attendee4@example.com3"),
                                                       Akonadi::SearchTerm::CondEqual));
            QVector<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find events tentative") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus,
                                                       QStringLiteral("attendee5@example.com4"),
                                                       Akonadi::SearchTerm::CondEqual));
            QVector<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find events delegated") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus,
                                                       QStringLiteral("attendee5@example.com5"),
                                                       Akonadi::SearchTerm::CondEqual));
            QVector<qint64> collections({6});
            QSet<qint64> result;
            QTest::newRow("unknown partstatus") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Summary, QStringLiteral("title"), Akonadi::SearchTerm::CondEqual));
            QVector<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find event summary") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Location, QStringLiteral("here"), Akonadi::SearchTerm::CondEqual));
            QVector<qint64> collections({6});
            QSet<qint64> result({2001});
            QTest::newRow("find events location") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
    }

    void testCalendarSearch()
    {
        resultSearch();
    }

    void testNoteSearch_data()
    {
        QTest::addColumn<QString>("query");
        QTest::addColumn<QVector<qint64>>("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64>>("expectedResult");
        const QStringList notesMimeTypes({QStringLiteral("text/x-vnd.akonadi.note")});
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note"), Akonadi::SearchTerm::CondEqual));

            QVector<qint64> collections({5});
            QSet<qint64> result({1000});
            QTest::newRow("find note subject equal") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note1"), Akonadi::SearchTerm::CondEqual));

            QVector<qint64> collections({5});
            QSet<qint64> result;
            QTest::newRow("find note subject equal") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note"), Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("note"), Akonadi::SearchTerm::CondEqual));

            QVector<qint64> collections({5});
            QSet<qint64> result({1000, 1001});
            QTest::newRow("find note subject equal or body equal") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note3"), Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("note3"), Akonadi::SearchTerm::CondEqual));

            QVector<qint64> collections({5});
            QSet<qint64> result({1002});
            QTest::newRow("find note subject equal and body equal") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            Akonadi::EmailSearchTerm term(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note3"), Akonadi::SearchTerm::CondEqual);
            term.setIsNegated(true);
            query.addTerm(term);
            QVector<qint64> collections({5});
            QSet<qint64> result({1000, 1001});
            QTest::newRow("find not subject equal note3") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
    }

    void testNoteSearch()
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
        QTest::addColumn<QVector<qint64>>("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64>>("expectedResult");
        const QStringList contactMimeTypes({KContacts::Addressee::mimeType()});
        const QStringList contactGroupMimeTypes({KContacts::ContactGroup::mimeType()});
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("John"), Akonadi::SearchTerm::CondContains));

            QVector<qint64> collections;
            QSet<qint64> result({100});
            QTest::newRow("contact by name") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("John"), Akonadi::SearchTerm::CondContains));

            QVector<qint64> collections({4});
            QSet<qint64> result;
            QTest::newRow("contact collectionfilter") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("john"), Akonadi::SearchTerm::CondContains));

            QVector<qint64> collections({3});
            QSet<qint64> result({100});
            QTest::newRow("contact by lowercase name") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Nickname, QStringLiteral("JD"), Akonadi::SearchTerm::CondContains));

            QVector<qint64> collections({3});
            QSet<qint64> result({100});
            QTest::newRow("contact by nickname") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Uid, QStringLiteral("uid1"), Akonadi::SearchTerm::CondEqual));

            QVector<qint64> collections({3});
            QSet<qint64> result({100});
            QTest::newRow("contact by uid") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Uid, QStringLiteral("abcd-efgh-1234-5678"), Akonadi::SearchTerm::CondEqual));

            QVector<qint64> collections({3});
            QSet<qint64> result({105});
            QTest::newRow("contact by uid 2") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, QStringLiteral("JANE@TEST.COM"), Akonadi::SearchTerm::CondEqual));

            QVector<qint64> collections({3});
            QSet<qint64> result({101, 102});
            QTest::newRow("contact by email") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Doe"), Akonadi::SearchTerm::CondContains));

            QVector<qint64> collections;
            QSet<qint64> result({100, 101, 102});
            QTest::newRow("contact by name (Doe)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Do"), Akonadi::SearchTerm::CondContains));

            QVector<qint64> collections;
            QSet<qint64> result({100, 101, 102});
            QTest::newRow("contact by name (Do)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("group1"), Akonadi::SearchTerm::CondContains));

            QVector<qint64> collections;
            QSet<qint64> result({103});
            QTest::newRow("contact group by name (group1)") << QString::fromLatin1(query.toJSON()) << collections << contactGroupMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("group2"), Akonadi::SearchTerm::CondContains));

            QVector<qint64> collections;
            QSet<qint64> result;
            QTest::newRow("contact group by name (group2)") << QString::fromLatin1(query.toJSON()) << collections << contactGroupMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("group3"), Akonadi::SearchTerm::CondContains));

            QVector<qint64> collections({4});
            QSet<qint64> result({104});
            QTest::newRow("contact group by name (group3 in collection 4)")
                << QString::fromLatin1(query.toJSON()) << collections << contactGroupMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("group3"), Akonadi::SearchTerm::CondContains));

            QVector<qint64> collections({3});
            QSet<qint64> result;
            QTest::newRow("contact group by name (group3 in collection 3)")
                << QString::fromLatin1(query.toJSON()) << collections << contactGroupMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("oe"), Akonadi::SearchTerm::CondContains));

            QVector<qint64> collections;
            QSet<qint64> result({100, 101, 102});
            QTest::newRow("contact by name (oe)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
    }

    void testEmailSearch_data()
    {
        QTest::addColumn<QString>("query");
        QTest::addColumn<QVector<qint64>>("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64>>("expectedResult");
        const QStringList emailMimeTypes = QStringList() << QStringLiteral("message/rfc822");
        const QVector<qint64> allEmailCollections({1, 2});
        {
            Akonadi::SearchQuery query;
            QVector<qint64> collections({1});
            QSet<qint64> result = {1};
            QTest::newRow("all emails in collection") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1"), Akonadi::SearchTerm::CondEqual));
            QVector<qint64> collections({1});
            QSet<qint64> result({1});
            QTest::newRow("find subject equal") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            Akonadi::EmailSearchTerm term(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1"), Akonadi::SearchTerm::CondEqual);
            term.setIsNegated(true);
            query.addTerm(term);
            QVector<qint64> collections({2});
            QSet<qint64> result({2, 3, 4, 5, 6});
            QTest::newRow("find subject equal negated") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1, 2, 3, 4});
            QTest::newRow("find subject contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("body"), Akonadi::SearchTerm::CondContains));
            QVector<qint64> collections({1, 2, 3, 4});
            QSet<qint64> result({1, 2, 3, 4, 6});
            QTest::newRow("find body contains") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("mälmöö"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1});
            QTest::newRow("find utf8 body contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Headers, QStringLiteral("From:"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1, 2, 3, 4, 5, 6});
            QTest::newRow("find header contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Message, QStringLiteral("body"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1, 2, 3, 4, 6});
            QTest::newRow("find message contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1"), Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject2"), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result({1, 2});
            QTest::newRow("or term") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1"), Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("body1"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1});
            QTest::newRow("and term") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1"), Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("body2"), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result;
            QTest::newRow("and term equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject"), Akonadi::SearchTerm::CondContains));
            QVector<qint64> collections({1});
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
            query.addTerm(
                Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, QStringLiteral("Jane Doe <cc@test.com>"), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result({4});
            QTest::newRow("find by header cc") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, QStringLiteral("cc@test.com"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({4});
            QTest::newRow("find by header cc (contains)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOrganization, QStringLiteral("kde"), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result({2});
            QTest::newRow("find by header organization (equal)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOrganization, QStringLiteral("kde"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({2, 3});
            QTest::newRow("find by header organization (contains)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(
                Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderBCC, QStringLiteral("Jane Doe <bcc@test.com>"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({4});
            QTest::newRow("find by header bcc") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderReplyTo, QStringLiteral("test@kde.org"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({4});
            QTest::newRow("find by reply to") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(
                Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, QStringLiteral("kde-pim.kde.org"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({4});
            QTest::newRow("find by list id") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Deleted),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(
                Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, QStringLiteral("kde-pim.kde.org"), Akonadi::SearchTerm::CondContains));

            QSet<qint64> result({4, 5});
            QTest::newRow("find by message by deleted status or headerListId")
                << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Deleted),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(
                Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, QStringLiteral("kde-pim.kde.org"), Akonadi::SearchTerm::CondContains));

            QVector<qint64> collections;
            QSet<qint64> result({4, 5});
            QTest::newRow("find by message by deleted status or headerListId in all collections")
                << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus,
                                                   QString::fromLatin1(Akonadi::MessageFlags::Deleted),
                                                   Akonadi::SearchTerm::CondContains));
            query.addTerm(
                Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, QStringLiteral("kde-pim.kde.org"), Akonadi::SearchTerm::CondContains));

            QSet<qint64> result;
            QTest::newRow("find by message by deleted status and headerListId")
                << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Message, QStringLiteral("subject"), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result({1, 2, 3, 4, 5, 6});
            QTest::newRow("find by message term") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, QStringLiteral("CC@TEST.com"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({4});
            QTest::newRow("find by header cc (contains) with case") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            //Change in qt/qtx11extras[stable]: remove QtWidgets dependency
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("extras"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({6});
            QTest::newRow("search extras in subject") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            // Change in qt/qtx11extras[stable]: remove QtWidgets dependency
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("change"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({6});
            QTest::newRow("search \"change\" in subject") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            // Change in qt/qtx11extras[stable]: remove QtWidgets dependency
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("qtx11extras"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({6});
            QTest::newRow("search qtx11extras in subject") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderFrom, QStringLiteral("test.com"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result({1, 2, 3, 4, 5, 6});
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
