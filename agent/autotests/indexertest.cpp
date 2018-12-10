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
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <QTest>
#include <AkonadiCore/Collection>
#include <KContacts/Addressee>
#include <QDir>

#include "emailindexer.h"
#include "contactindexer.h"
#include <../search/email/emailsearchstore.h>
#include <../search/contact/contactsearchstore.h>
#include <query.h>

Q_DECLARE_METATYPE(QSet<qint64>)
Q_DECLARE_METATYPE(QList<qint64>)

class IndexerTest : public QObject
{
    Q_OBJECT
private:
    QString emailDir;
    QString emailContactsDir;
    QString contactsDir;
    QString notesDir;

    bool removeDir(const QString &dirName)
    {
        bool result = true;
        QDir dir(dirName);

        if (dir.exists(dirName)) {
            Q_FOREACH (const QFileInfo &info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
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
        emailDir = QDir::tempPath() + QLatin1String("/searchplugintest/email/");
        emailContactsDir = QDir::tempPath() + QLatin1String("/searchplugintest/emailcontacts/");
        contactsDir = QDir::tempPath() + QLatin1String("/searchplugintest/contacts/");
        notesDir = QDir::tempPath() + QStringLiteral("/searchplugintest/notes/");

        QDir dir;
        removeDir(emailDir);
        QVERIFY(dir.mkpath(emailDir));
        removeDir(emailContactsDir);
        QVERIFY(dir.mkpath(emailContactsDir));
        removeDir(contactsDir);
        QVERIFY(dir.mkpath(contactsDir));
        removeDir(notesDir);
        QVERIFY(dir.mkpath(notesDir));

        qDebug() << "indexing sample data";
        qDebug() << emailDir;
        qDebug() << emailContactsDir;
        qDebug() << notesDir;

//         EmailIndexer emailIndexer(emailDir, emailContactsDir);
//         ContactIndexer contactIndexer(contactsDir);

//         Akonadi::Search::EmailSearchStore *emailSearchStore = new Akonadi::Search::EmailSearchStore(this);
//         emailSearchStore->setDbPath(emailDir);
//         Akonadi::Search::ContactSearchStore *contactSearchStore = new Akonadi::Search::ContactSearchStore(this);
//         contactSearchStore->setDbPath(contactsDir);
//         Akonadi::Search::SearchStore::overrideSearchStores(QList<Akonadi::Search::SearchStore*>() << emailSearchStore << contactSearchStore);
    }

    QSet<qint64> getAllItems()
    {
        QSet<qint64> resultSet;

        Akonadi::Search::Term term(Akonadi::Search::Term::Or);
        term.addSubTerm(Akonadi::Search::Term(QStringLiteral("collection"), QStringLiteral("1"), Akonadi::Search::Term::Equal));
        term.addSubTerm(Akonadi::Search::Term(QStringLiteral("collection"), QStringLiteral("2"), Akonadi::Search::Term::Equal));
        Akonadi::Search::Query query(term);
        query.setType(QStringLiteral("Email"));

        Akonadi::Search::EmailSearchStore *emailSearchStore = new Akonadi::Search::EmailSearchStore(this);
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

    void testEmailRemoveByCollection()
    {
        EmailIndexer emailIndexer(emailDir, emailContactsDir);
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("subject1");
            msg->assemble();

            Akonadi::Item item(QStringLiteral("message/rfc822"));
            item.setId(1);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(1));
            emailIndexer.index(item);
        }
        {
            KMime::Message::Ptr msg(new KMime::Message);
            msg->subject()->from7BitString("subject2");
            msg->assemble();

            Akonadi::Item item(QStringLiteral("message/rfc822"));
            item.setId(2);
            item.setPayload(msg);
            item.setParentCollection(Akonadi::Collection(2));
            emailIndexer.index(item);
        }
        emailIndexer.commit();
        QCOMPARE(getAllItems(), QSet<qint64>() << 1 << 2);
        emailIndexer.remove(Akonadi::Collection(2));
        emailIndexer.commit();
        QCOMPARE(getAllItems(), QSet<qint64>() << 1);
    }
};

QTEST_GUILESS_MAIN(IndexerTest)

#include "indexertest.moc"

