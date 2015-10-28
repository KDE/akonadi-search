/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2014  Christian Mollekopf <mollekopf@kolabsys.com>
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

#include <QTest>
#include <AkonadiCore/Collection>
#include <KContacts/Addressee>
#include <KContacts/ContactGroup>
#include <QDir>

#include "searchplugin.h"
#include <../agent/emailindexer.h>
#include <../agent/contactindexer.h>
#include <../agent/calendarindexer.h>
#include <../agent/akonotesindexer.h>
#include <../search/email/emailsearchstore.h>
#include <../search/contact/contactsearchstore.h>
#include <../search/calendar/calendarsearchstore.h>
#include <../search/note/notesearchstore.h>
#include <AkonadiCore/searchquery.h>
#include <Akonadi/KMime/MessageFlags>

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
    QString noteDir;
    QString calendarDir;

    bool removeDir(const QString &dirName)
    {
        bool result = true;
        QDir dir(dirName);

        if (dir.exists(dirName)) {
            Q_FOREACH (QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
                if (info.isDir()) {
                    result = removeDir(info.absoluteFilePath());
                } else {
                    result = QFile::remove(info.absoluteFilePath());
                }

                if (!result) {
                    return result;
                }
            }
            result = dir.rmdir(dirName);
        }
        return result;
    }
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
        QCOMPARE(result, expectedResult);
    }

private Q_SLOTS:
    void init()
    {
        emailDir = QDir::tempPath() + QLatin1String("/searchplugintest/email/");
        emailContactsDir = QDir::tempPath() + QLatin1String("/searchplugintest/emailcontacts/");
        contactsDir = QDir::tempPath() + QLatin1String("/searchplugintest/contacts/");
        noteDir = QDir::tempPath() + QLatin1String("/searchplugintest/notes/");
        calendarDir = QDir::tempPath() + QLatin1String("/searchplugintest/calendar/");

        QDir dir;
        QVERIFY(removeDir(emailDir));
        QVERIFY(dir.mkpath(emailDir));
        QVERIFY(removeDir(emailContactsDir));
        QVERIFY(dir.mkpath(emailContactsDir));
        QVERIFY(removeDir(contactsDir));
        QVERIFY(dir.mkpath(contactsDir));
        QVERIFY(removeDir(noteDir));
        QVERIFY(dir.mkpath(noteDir));
        QVERIFY(removeDir(calendarDir));
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

            //Multipart message
            KMime::Content *b = new KMime::Content;
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
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Seen
                          << Akonadi::MessageFlags::Deleted
                          << Akonadi::MessageFlags::Answered
                          << Akonadi::MessageFlags::Flagged
                          << Akonadi::MessageFlags::HasAttachment
                          << Akonadi::MessageFlags::HasInvitation
                          << Akonadi::MessageFlags::Sent
                          //<< Akonadi::MessageFlags::Queued    //can't have Sent and Queued at the same time
                          << Akonadi::MessageFlags::Replied
                          << Akonadi::MessageFlags::Forwarded
                          << Akonadi::MessageFlags::ToAct
                          << Akonadi::MessageFlags::Watched
                          //<< Akonadi::MessageFlags::Ignored   // can't have Watched and Ignored at the same time
                          << Akonadi::MessageFlags::Encrypted
                          /*<< Akonadi::MessageFlags::Spam*/
                          << Akonadi::MessageFlags::Ham);
            //Spam is exclude from indexer. So we can't add it.
            emailIndexer.index(item);
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

            Akonadi::Item item(QStringLiteral("message/rfc822"));
            item.setId(6);
            item.setSize(50);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            item.setFlags(Akonadi::Item::Flags() << Akonadi::MessageFlags::Flagged << Akonadi::MessageFlags::Replied);
            emailIndexer.index(item);
        }
        emailIndexer.commit();

        //Contact item
        {
            KContacts::Addressee addressee;
            addressee.setUid(QStringLiteral("uid1"));
            addressee.setName(QStringLiteral("John Doe"));
            addressee.setFormattedName(QStringLiteral("John Doe"));
            addressee.setNickName(QStringLiteral("JD"));
            addressee.setEmails(QStringList() << QStringLiteral("john@test.com"));
            addressee.setBirthday(QDateTime(QDate(2000, 01, 01)));
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
            addressee.setEmails(QStringList() << QStringLiteral("jane@test.com"));
            addressee.setBirthday(QDateTime(QDate(2001, 01, 01)));
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
            addressee.setEmails(QStringList() << QStringLiteral("JANE@TEST.COM"));
            addressee.setBirthday(QDateTime(QDate(2001, 01, 01)));
            Akonadi::Item item(KContacts::Addressee::mimeType());
            item.setId(102);
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

        //Note item
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("note");

            //Multipart message
            KMime::Content *b = new KMime::Content;
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

            //Multipart message
            KMime::Content *b = new KMime::Content;
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

            //Multipart message
            KMime::Content *b = new KMime::Content;
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
            KCalCore::Event::Ptr event(new KCalCore::Event);
            KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee(QStringLiteral("attendee1"), QStringLiteral("attendee1@example.com"), false, KCalCore::Attendee::NeedsAction));
            event->setOrganizer(QStringLiteral("organizer@example.com"));
            event->addAttendee(attendee);
            attendee = KCalCore::Attendee::Ptr(new KCalCore::Attendee(QStringLiteral("attendee2"), QStringLiteral("attendee2@example.com"), false, KCalCore::Attendee::Accepted));
            event->addAttendee(attendee);
            attendee = KCalCore::Attendee::Ptr(new KCalCore::Attendee(QStringLiteral("attendee3"), QStringLiteral("attendee3@example.com"), false, KCalCore::Attendee::Declined));
            event->addAttendee(attendee);
            attendee = KCalCore::Attendee::Ptr(new KCalCore::Attendee(QStringLiteral("attendee4"), QStringLiteral("attendee4@example.com"), false, KCalCore::Attendee::Tentative));
            event->addAttendee(attendee);
            attendee = KCalCore::Attendee::Ptr(new KCalCore::Attendee(QStringLiteral("attendee5"), QStringLiteral("attendee5@example.com"), false, KCalCore::Attendee::Delegated));
            event->addAttendee(attendee);

            event->setSummary(QStringLiteral("title"));
            event->setLocation(QStringLiteral("here"));

            Akonadi::Item item(KCalCore::Event::eventMimeType());
            item.setId(2001);
            item.setPayload<KCalCore::Event::Ptr>(event);
            item.setParentCollection(Akonadi::Collection(6));
            calendarIndexer.index(item);
        }
        calendarIndexer.commit();

        Akonadi::Search::EmailSearchStore *emailSearchStore = new Akonadi::Search::EmailSearchStore();
        emailSearchStore->setDbPath(emailDir);
        Akonadi::Search::ContactSearchStore *contactSearchStore = new Akonadi::Search::ContactSearchStore();
        contactSearchStore->setDbPath(contactsDir);
        Akonadi::Search::NoteSearchStore *noteSearchStore = new Akonadi::Search::NoteSearchStore();
        noteSearchStore->setDbPath(noteDir);
        Akonadi::Search::CalendarSearchStore *calendarSearchStore = new Akonadi::Search::CalendarSearchStore();
        calendarSearchStore->setDbPath(calendarDir);

        Akonadi::Search::SearchStore::overrideSearchStores(QList<Akonadi::Search::SearchStore *>() << emailSearchStore << contactSearchStore << noteSearchStore << calendarSearchStore);
    }

    void testCalendarSearch_data()
    {
        QTest::addColumn<QString>("query");
        QTest::addColumn<QList<qint64> >("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64> >("expectedResult");
        const QStringList calendarMimeTypes = QStringList() << KCalCore::Event::eventMimeType();
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Organizer, "organizer@example.com", Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 6;
            QSet<qint64> result = QSet<qint64>() << 2001;
            QTest::newRow("find organizer") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Organizer, "organizer2@example.com", Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 6;
            QSet<qint64> result;
            QTest::newRow("find no organizer") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, "attendee1@example.com0", Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections = QList<qint64>() << 6;
            QSet<qint64> result = QSet<qint64>() << 2001;
            QTest::newRow("find events needsAction") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, "attendee2@example.com1", Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections = QList<qint64>() << 6;
            QSet<qint64> result = QSet<qint64>() << 2001;
            QTest::newRow("find events accepted") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, "attendee3@example.com2", Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections = QList<qint64>() << 6;
            QSet<qint64> result = QSet<qint64>() << 2001;
            QTest::newRow("find events declined") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, "attendee4@example.com3", Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections = QList<qint64>() << 6;
            QSet<qint64> result = QSet<qint64>() << 2001;
            QTest::newRow("find events tentative") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, "attendee5@example.com4", Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections = QList<qint64>() << 6;
            QSet<qint64> result = QSet<qint64>() << 2001;
            QTest::newRow("find events delegated") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, "attendee5@example.com5", Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections = QList<qint64>() << 6;
            QSet<qint64> result;
            QTest::newRow("unknown partstatus") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Summary, "title", Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections = QList<qint64>() << 6;
            QSet<qint64> result = QSet<qint64>() << 2001;
            QTest::newRow("find event summary") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::Location, "here", Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections = QList<qint64>() << 6;
            QSet<qint64> result = QSet<qint64>() << 2001;
            QTest::newRow("find events location") << QString::fromLatin1(query.toJSON()) << collections << calendarMimeTypes << result;
        }
    }

    void testCalendarSearch()
    {
        resultSearch();
    }

#if 1
    void testNoteSearch_data()
    {
        QTest::addColumn<QString>("query");
        QTest::addColumn<QList<qint64> >("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64> >("expectedResult");
        const QStringList notesMimeTypes = QStringList() << QStringLiteral("text/x-vnd.akonadi.note");
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note"), Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 5;
            QSet<qint64> result = QSet<qint64>() << 1000;
            QTest::newRow("find note subject equal") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note1"), Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 5;
            QSet<qint64> result ;
            QTest::newRow("find note subject equal") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note"), Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("note"), Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 5;
            QSet<qint64> result = QSet<qint64>() << 1000 << 1001;
            QTest::newRow("find note subject equal or body equal") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note3"), Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("note3"), Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 5;
            QSet<qint64> result = QSet<qint64>() << 1002;
            QTest::newRow("find note subject equal and body equal") << QString::fromLatin1(query.toJSON()) << collections << notesMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            Akonadi::EmailSearchTerm term(Akonadi::EmailSearchTerm::Subject, QStringLiteral("note3"), Akonadi::SearchTerm::CondEqual);
            term.setIsNegated(true);
            query.addTerm(term);
            QList<qint64> collections = QList<qint64>() << 5;
            QSet<qint64> result = QSet<qint64>() << 1000 << 1001;
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
        QTest::addColumn<QList<qint64> >("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64> >("expectedResult");
        const QStringList contactMimeTypes = QStringList() << KContacts::Addressee::mimeType();
        const QStringList contactGroupMimeTypes = QStringList() << KContacts::ContactGroup::mimeType();
#if 1
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("John"), Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result = QSet<qint64>() << 100;
            QTest::newRow("contact by name") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("John"), Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 4;
            QSet<qint64> result;
            QTest::newRow("contact collectionfilter") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("john"), Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 3;
            QSet<qint64> result = QSet<qint64>() << 100;
            QTest::newRow("contact by lowercase name") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Nickname, QStringLiteral("JD"), Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 3;
            QSet<qint64> result = QSet<qint64>() << 100;
            QTest::newRow("contact by nickname") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Uid, QStringLiteral("uid1"), Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 3;
            QSet<qint64> result = QSet<qint64>() << 100;
            QTest::newRow("contact by uid") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Email, QStringLiteral("JANE@TEST.COM"), Akonadi::SearchTerm::CondEqual));

            QList<qint64> collections = QList<qint64>() << 3;
            QSet<qint64> result = QSet<qint64>() << 101 << 102;
            QTest::newRow("contact by email") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Doe"), Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result = QSet<qint64>() << 100 << 101 << 102;
            QTest::newRow("contact by name (Doe)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("Do"), Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result = QSet<qint64>() << 100 << 101 << 102;
            QTest::newRow("contact by name (Do)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
#endif
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("group1"), Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result = QSet<qint64>() << 103;
            QTest::newRow("contact group by name (group1)") << QString::fromLatin1(query.toJSON()) << collections << contactGroupMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("group2"), Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result;
            QTest::newRow("contact group by name (group2)") << QString::fromLatin1(query.toJSON()) << collections << contactGroupMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("group3"), Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 4;
            QSet<qint64> result = QSet<qint64>() << 104;
            QTest::newRow("contact group by name (group3 in collection 4)") << QString::fromLatin1(query.toJSON()) << collections << contactGroupMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("group3"), Akonadi::SearchTerm::CondContains));

            QList<qint64> collections = QList<qint64>() << 3;
            QSet<qint64> result;
            QTest::newRow("contact group by name (group3 in collection 3)") << QString::fromLatin1(query.toJSON()) << collections << contactGroupMimeTypes << result;
        }

#if 0 //Doesn't work for the moment
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::ContactSearchTerm(Akonadi::ContactSearchTerm::Name, QStringLiteral("oe"), Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result = QSet<qint64>() << 100 << 101 << 102;
            QTest::newRow("contact by name (oe)") << QString::fromLatin1(query.toJSON()) << collections << contactMimeTypes << result;
        }
#endif
    }
#endif
    void testEmailSearch_data()
    {
        QTest::addColumn<QString>("query");
        QTest::addColumn<QList<qint64> >("collections");
        QTest::addColumn<QStringList>("mimeTypes");
        QTest::addColumn<QSet<qint64> >("expectedResult");
        const QStringList emailMimeTypes = QStringList() << QStringLiteral("message/rfc822");
        const QList<qint64> allEmailCollections = QList<qint64>() << 1 << 2;
#if 1
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1"), Akonadi::SearchTerm::CondEqual));
            QList<qint64> collections = QList<qint64>() << 1;
            QSet<qint64> result = QSet<qint64>() << 1;
            QTest::newRow("find subject equal") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            Akonadi::EmailSearchTerm term(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1"), Akonadi::SearchTerm::CondEqual);
            term.setIsNegated(true);
            query.addTerm(term);
            QList<qint64> collections = QList<qint64>() << 2;
            QSet<qint64> result = QSet<qint64>() << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find subject equal negated") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4;
            QTest::newRow("find subject contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("body"), Akonadi::SearchTerm::CondContains));
            QList<qint64> collections = QList<qint64>() << 1 << 2 << 3 << 4;
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 6;
            QTest::newRow("find body contains") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("mälmöö"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 1;
            QTest::newRow("find utf8 body contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Headers, QStringLiteral("From:"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find header contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Message, QStringLiteral("body"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 6;
            QTest::newRow("find message contains") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1"), Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject2"), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result = QSet<qint64>() << 1 << 2;
            QTest::newRow("or term") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("subject1"), Akonadi::SearchTerm::CondEqual));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Body, QStringLiteral("body1"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 1;
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
            QList<qint64> collections = QList<qint64>() << 1;
            QSet<qint64> result = QSet<qint64>() << 1;
            QTest::newRow("filter by collection") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Flagged), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by message flag") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Replied), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by message replied") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Replied), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Encrypted), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 1 << 5;
            QTest::newRow("find by message replied and encrypted") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Seen), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Deleted), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Answered), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Flagged), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::HasAttachment), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::HasInvitation), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Sent), Akonadi::SearchTerm::CondContains));
            //query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Queued), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Replied), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Forwarded), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::ToAct), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Watched), Akonadi::SearchTerm::CondContains));
            //query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Ignored), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Encrypted), Akonadi::SearchTerm::CondContains));
            //Spam is exclude from indexer.
            //query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Spam), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Ham), Akonadi::SearchTerm::CondContains));

            QSet<qint64> result = QSet<qint64>() << 5;
            QTest::newRow("find by message by all status") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1000), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by size greater than equal great or equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1000), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result = QSet<qint64>() << 1;
            QTest::newRow("find by size equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1002), Akonadi::SearchTerm::CondLessOrEqual));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 5;
            QTest::newRow("find by size greater than equal less or equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1001), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result = QSet<qint64>() << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by size separate") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::ByteSize, QString::number(1001), Akonadi::SearchTerm::CondGreaterThan));
            QSet<qint64> result = QSet<qint64>() << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by size separate (greater than)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderDate, QDateTime(QDate(2013, 11, 10), QTime(12, 30, 0)), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result = QSet<qint64>() << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by date") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderDate, QDateTime(QDate(2013, 11, 10), QTime(12, 0, 0)), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result = QSet<qint64>() << 1;
            QTest::newRow("find by date equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2014, 11, 11), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result = QSet<qint64>() << 4 << 5 << 6;
            QTest::newRow("find by date only (equal condition)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2013, 11, 10), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by date only (greater or equal)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2014, 11, 10), Akonadi::SearchTerm::CondGreaterOrEqual));
            QSet<qint64> result = QSet<qint64>() << 3 << 4  << 5 << 6;
            QTest::newRow("find by date only greater or equal") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOnlyDate, QDate(2014, 11, 10), Akonadi::SearchTerm::CondGreaterThan));
            QSet<qint64> result = QSet<qint64>() << 4 << 5 << 6;
            QTest::newRow("find by date only greater than") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, QStringLiteral("Jane Doe <cc@test.com>"), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result = QSet<qint64>() << 4;
            QTest::newRow("find by header cc") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, QStringLiteral("cc@test.com"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 4;
            QTest::newRow("find by header cc (contains)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }

        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOrganization, QStringLiteral("kde"), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result = QSet<qint64>() << 2;
            QTest::newRow("find by header organization (equal)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderOrganization, QStringLiteral("kde"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 2 << 3;
            QTest::newRow("find by header organization (contains)") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderBCC, QStringLiteral("Jane Doe <bcc@test.com>"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 4;
            QTest::newRow("find by header bcc") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderReplyTo, QStringLiteral("test@kde.org"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 4;
            QTest::newRow("find by reply to") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, QStringLiteral("kde-pim.kde.org"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 4;
            QTest::newRow("find by list id") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Deleted), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, QStringLiteral("kde-pim.kde.org"), Akonadi::SearchTerm::CondContains));

            QSet<qint64> result = QSet<qint64>() << 4 << 5;
            QTest::newRow("find by message by deleted status or headerListId") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Deleted), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, QStringLiteral("kde-pim.kde.org"), Akonadi::SearchTerm::CondContains));

            QList<qint64> collections;
            QSet<qint64> result = QSet<qint64>() << 4 << 5;
            QTest::newRow("find by message by deleted status or headerListId in all collections") << QString::fromLatin1(query.toJSON()) << collections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query(Akonadi::SearchTerm::RelAnd);
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::MessageStatus, QString::fromLatin1(Akonadi::MessageFlags::Deleted), Akonadi::SearchTerm::CondContains));
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderListId, QStringLiteral("kde-pim.kde.org"), Akonadi::SearchTerm::CondContains));

            QSet<qint64> result;
            QTest::newRow("find by message by deleted status and headerListId") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Message, QStringLiteral("subject"), Akonadi::SearchTerm::CondEqual));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("find by message term") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderCC, QStringLiteral("CC@TEST.com"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 4;
            QTest::newRow("find by header cc (contains) with case") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
#if 0 //Can not work for the moment
        {
            Akonadi::SearchQuery query;
            //Change in qt/qtx11extras[stable]: remove QtWidgets dependency
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("extras"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 6;
            QTest::newRow("search extras in subject") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
#endif
        {
            Akonadi::SearchQuery query;
            //Change in qt/qtx11extras[stable]: remove QtWidgets dependency
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("change"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 6;
            QTest::newRow("search \"change\" in subject") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
        {
            Akonadi::SearchQuery query;
            //Change in qt/qtx11extras[stable]: remove QtWidgets dependency
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::Subject, QStringLiteral("qtx11extras"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 6;
            QTest::newRow("search qtx11extras in subject") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
#endif
        {
            Akonadi::SearchQuery query;
            query.addTerm(Akonadi::EmailSearchTerm(Akonadi::EmailSearchTerm::HeaderFrom, QStringLiteral("test.com"), Akonadi::SearchTerm::CondContains));
            QSet<qint64> result = QSet<qint64>() << 1 << 2 << 3 << 4 << 5 << 6;
            QTest::newRow("search by from email part") << QString::fromLatin1(query.toJSON()) << allEmailCollections << emailMimeTypes << result;
        }
    }

    void testEmailSearch()
    {
        resultSearch();
    }

};

QTEST_MAIN(SearchPluginTest)

#include "searchplugintest.moc"

