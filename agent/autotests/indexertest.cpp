/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include <Akonadi/Collection>
using namespace Qt::Literals::StringLiterals;

#include <QDir>
#include <QTest>

#include "../search/calendar/calendarsearchstore.h"
#include "../search/contact/contactsearchstore.h"
#include "../search/email/emailsearchstore.h"
#include "calendarindexer.h"
#include "contactindexer.h"
#include "emailindexer.h"
#include "query.h"

Q_DECLARE_METATYPE(QSet<qint64>)
Q_DECLARE_METATYPE(QList<qint64>)

static std::shared_ptr<KMime::Message> readMailFromFile(const QString &mailFile)
{
    QFile file(QLatin1StringView(MAIL_DATA_DIR) + u'/' + mailFile);
    const auto ok = file.open(QIODevice::ReadOnly);
    Q_ASSERT(ok && file.isOpen());
    auto mailData = KMime::CRLFtoLF(file.readAll());
    std::shared_ptr<KMime::Message> message(new KMime::Message);
    message->setContent(mailData);
    message->parse();
    return message;
}

class IndexerTest : public QObject
{
    Q_OBJECT
private:
    QString emailDir;
    QString emailContactsDir;
    QString contactsDir;
    QString calendarsDir;
    QString notesDir;

    bool removeDir(const QString &dirName)
    {
        bool result = true;
        QDir dir(dirName);

        if (dir.exists(dirName)) {
            const QFileInfoList infoDirs = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
            for (const QFileInfo &info : infoDirs) {
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

private Q_SLOTS:
    void init()
    {
        emailDir = QDir::tempPath() + "/searchplugintest/email/"_L1;
        emailContactsDir = QDir::tempPath() + "/searchplugintest/emailcontacts/"_L1;
        contactsDir = QDir::tempPath() + "/searchplugintest/contacts/"_L1;
        notesDir = QDir::tempPath() + u"/searchplugintest/notes/"_s;
        calendarsDir = QDir::tempPath() + u"/searchplugintest/calendars/"_s;

        QDir dir;
        removeDir(emailDir);
        QVERIFY(dir.mkpath(emailDir));
        removeDir(emailContactsDir);
        QVERIFY(dir.mkpath(emailContactsDir));
        removeDir(contactsDir);
        QVERIFY(dir.mkpath(contactsDir));
        removeDir(notesDir);
        QVERIFY(dir.mkpath(notesDir));
        removeDir(calendarsDir);
        QVERIFY(dir.mkpath(calendarsDir));

        qDebug() << "indexing sample data";
        qDebug() << emailDir;
        qDebug() << emailContactsDir;
        qDebug() << notesDir;
        qDebug() << calendarsDir;

        //         EmailIndexer emailIndexer(emailDir, emailContactsDir);
        //         ContactIndexer contactIndexer(contactsDir);

        //         Akonadi::Search::EmailSearchStore *emailSearchStore = new Akonadi::Search::EmailSearchStore(this);
        //         emailSearchStore->setDbPath(emailDir);
        //         Akonadi::Search::ContactSearchStore *contactSearchStore = new Akonadi::Search::ContactSearchStore(this);
        //         contactSearchStore->setDbPath(contactsDir);
        //         Akonadi::Search::SearchStore::overrideSearchStores(QList<Akonadi::Search::SearchStore*>() << emailSearchStore << contactSearchStore);
    }

    QSet<qint64> getAllEmailItems()
    {
        QSet<qint64> resultSet;

        Akonadi::Search::Term term(Akonadi::Search::Term::Or);
        term.addSubTerm(Akonadi::Search::Term(u"collection"_s, u"1"_s, Akonadi::Search::Term::Equal));
        term.addSubTerm(Akonadi::Search::Term(u"collection"_s, u"2"_s, Akonadi::Search::Term::Equal));
        Akonadi::Search::Query query(term);
        query.setType(u"Email"_s);

        auto emailSearchStore = new Akonadi::Search::EmailSearchStore(this);
        emailSearchStore->setDbPath(emailDir);
        int res = emailSearchStore->exec(query);
        qDebug() << res;
        while (emailSearchStore->next(res)) {
            const int fid = Akonadi::Search::deserialize("akonadi", emailSearchStore->id(res));
            resultSet << fid;
        }
        qDebug() << resultSet;
        return resultSet;
    }

    QSet<qint64> getAllCalendarItems()
    {
        QSet<qint64> resultSet;

        Akonadi::Search::Term term(Akonadi::Search::Term::Or);
        term.addSubTerm(Akonadi::Search::Term(u"collection"_s, u"1"_s, Akonadi::Search::Term::Equal));
        term.addSubTerm(Akonadi::Search::Term(u"collection"_s, u"2"_s, Akonadi::Search::Term::Equal));
        Akonadi::Search::Query query(term);
        query.setType(u"Calendar"_s);

        auto calendarSearchStore = new Akonadi::Search::CalendarSearchStore(this);
        calendarSearchStore->setDbPath(calendarsDir);
        int res = calendarSearchStore->exec(query);
        qDebug() << res;
        while (calendarSearchStore->next(res)) {
            const int fid = Akonadi::Search::deserialize("akonadi", calendarSearchStore->id(res));
            resultSet << fid;
        }
        qDebug() << resultSet;
        return resultSet;
    }

    void testEmailRemoveByCollection()
    {
        EmailIndexer emailIndexer(emailDir, emailContactsDir);
        {
            auto msg = std::make_shared<KMime::Message>();
            msg->subject()->from7BitString("subject1");
            msg->assemble();

            Akonadi::Item item(KMime::Message::mimeType());
            item.setId(1);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(1));
            emailIndexer.index(item);
        }
        {
            auto msg = std::make_shared<KMime::Message>();
            msg->subject()->from7BitString("subject2");
            msg->assemble();

            Akonadi::Item item(KMime::Message::mimeType());
            item.setId(2);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            emailIndexer.index(item);
        }
        emailIndexer.commit();
        QCOMPARE(getAllEmailItems(), QSet<qint64>() << 1 << 2);
        emailIndexer.remove(Akonadi::Collection(2));
        emailIndexer.commit();
        QCOMPARE(getAllEmailItems(), QSet<qint64>() << 1);
    }

    void testHtmlOnly()
    {
        auto msg = readMailFromFile(u"htmlonly.mbox"_s);

        Akonadi::Item item(KMime::Message::mimeType());
        item.setId(1);
        item.setPayload(msg);
        item.setParentCollection(Akonadi::Collection(1));

        EmailIndexer emailIndexer(emailDir, emailContactsDir);
        emailIndexer.index(item);
        emailIndexer.commit();
        QCOMPARE(getAllEmailItems(), QSet<qint64>() << 1);
    }

    void testCalendarRemoveByCollection()
    {
        CalendarIndexer calendarIndexer(calendarsDir);
        {
            KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
            event->setSummary(u"My Event 1"_s);

            Akonadi::Item item(KCalendarCore::Event::eventMimeType());
            item.setId(3);
            item.setPayload(event);
            item.setParentCollection(Akonadi::Collection(1));
            calendarIndexer.index(item);
        }

        {
            KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
            event->setSummary(u"My Event 2"_s);

            Akonadi::Item item(KCalendarCore::Event::eventMimeType());
            item.setId(4);
            item.setPayload(event);
            item.setParentCollection(Akonadi::Collection(2));
            calendarIndexer.index(item);
        }

        calendarIndexer.commit();
        QCOMPARE(getAllCalendarItems(), QSet<qint64>() << 3 << 4);

        calendarIndexer.remove(Akonadi::Collection(2));
        calendarIndexer.commit();
        QCOMPARE(getAllCalendarItems(), QSet<qint64>() << 3);
    }

    void testAddCalendarWithAttendee()
    {
        CalendarIndexer calendarIndexer(calendarsDir);
        {
            KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
            event->setSummary(u"My Event 1"_s);
            event->addAttendee(KCalendarCore::Attendee(u"John Doe"_s, u"john.doe@kmail.com"_s));

            Akonadi::Item item(KCalendarCore::Event::eventMimeType());
            item.setId(5);
            item.setPayload(event);
            item.setParentCollection(Akonadi::Collection(1));
            calendarIndexer.index(item);
        }
        calendarIndexer.commit();

        const auto status = KCalendarCore::Attendee::PartStat::NeedsAction;
        Akonadi::Search::Term term(Akonadi::Search::Term::Or);

        Akonadi::Search::Term partStatusTerm(u"partstatus"_s, QString(u"john.doe@kmail.com"_s + QString::number(status)), Akonadi::Search::Term::Equal);

        term.addSubTerm(partStatusTerm);

        Akonadi::Search::Query query(term);
        query.setType(u"Calendar"_s);

        auto calendarSearchStore = new Akonadi::Search::CalendarSearchStore(this);
        calendarSearchStore->setDbPath(calendarsDir);
        int res = calendarSearchStore->exec(query);
        qDebug() << res;
        while (calendarSearchStore->next(res)) {
            const int fid = Akonadi::Search::deserialize("akonadi", calendarSearchStore->id(res));
            QCOMPARE(fid, 5);
        }
    }
};

QTEST_GUILESS_MAIN(IndexerTest)

#include "indexertest.moc"
