/*
 * This file is part of the KDE Akonadi Search Project
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

#include <QObject>
#include <QTest>
#include <QStandardPaths>

#include <AkonadiCore/Item>
#include <AkonadiCore/Collection>
#include <AkonadiCore/SearchQuery>

#include <KMime/Message>
#include <KContacts/Addressee>

#include "indexingplugin.h"
#include "../../src/indexer.h"
#include "../../src/store.h"
#include "../../src/querymapper.h"
#include "../../src/resultiterator.h"

using namespace Akonadi::Search;

class IndexingPluginTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void cleanupTestCase()
    {
        QDir dir(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("akonadi/search_db"),
                                    QStandardPaths::LocateDirectory));
        QVERIFY(dir.absolutePath().contains(QLatin1String(".qttest")));
        if (dir.exists()) {
            dir.removeRecursively();
        }
    }

    QVector<qint64> searchCollection(const QString &mimeType, qint64 collectionId)
    {
        Akonadi::SearchTerm searchTerm(Akonadi::SearchTerm::Collection, collectionId, Akonadi::SearchTerm::CondEqual);
        Akonadi::SearchQuery query;
        query.setTerm(searchTerm);

        QScopedPointer<QueryMapper> qm(QueryMapper::create(mimeType));
        const QByteArray qb = qm->map(query);

        QScopedPointer<Store> store(Store::create(mimeType));
        auto result = store->search(qb, -1);
        QVector<qint64> rv;
        while (result.next()) {
            rv.push_back(result.id());
        }
        return rv;
    }

    void testIndexing()
    {
        auto plugin = new IndexingPlugin;
        QScopedPointer<Indexer> mimeIndexer(Indexer::create(KMime::Message::mimeType()));
        {
            auto msg = KMime::Message::Ptr::create();
            msg->subject(true)->from7BitString("Subject1");
            msg->from(true)->fromUnicodeString(QStringLiteral("Daniel Vrátil <dvratil@kde.org>"), "UTF-8");
            msg->to(true)->fromUnicodeString(QStringLiteral("Test Friend <friend@kde.test"), "UTF-8");
            msg->setBody("Hello Friend, how are you?");

            Akonadi::Item item(KMime::Message::mimeType());
            item.setId(1);
            item.setParentCollection(Akonadi::Collection(1));
            item.setPayload(msg);
            item.setSize(60);

            QVERIFY(plugin->index(item.mimeType(), item.id(), mimeIndexer->index(item)));
        }

        {
            auto msg = KMime::Message::Ptr::create();
            msg->subject(true)->from7BitString("Re: Subject1");
            msg->from(true)->fromUnicodeString(QStringLiteral("Test Friend <friend@kde.test"), "UTF-8");
            msg->to(true)->fromUnicodeString(QStringLiteral("Daniel Vrátil <dvratil@kde.org>"), "UTF-8");
            msg->setBody("Hello Dan, I'm fine. How are you?");

            Akonadi::Item item(KMime::Message::mimeType());
            item.setId(2);
            item.setParentCollection(Akonadi::Collection(1));
            item.setPayload(msg);
            item.setSize(70);

            QVERIFY(plugin->index(item.mimeType(), item.id(), mimeIndexer->index(item)));
        }

        delete plugin; // force commit

        QCOMPARE(searchCollection(KMime::Message::mimeType(), 1), (QVector<qint64>{ 1, 2 }));
    }

    void testItemMove()
    {
        QCOMPARE(searchCollection(KMime::Message::mimeType(), 1), (QVector<qint64>{ 1, 2 }));

        {
            IndexingPlugin plugin;
            // Move item 1 from collection 1 to collection 3
            plugin.move(KMime::Message::mimeType(), 1, 1, 3);
        }

        QCOMPARE(searchCollection(KMime::Message::mimeType(), 1), QVector<qint64>{ 2 });
        QCOMPARE(searchCollection(KMime::Message::mimeType(), 3), QVector<qint64>{ 1 }); 
    }

    void testItemRemove()
    {
        QCOMPARE(searchCollection(KMime::Message::mimeType(), 1), QVector<qint64>{ 2 });

        {
            IndexingPlugin plugin;
            plugin.removeItem(KMime::Message::mimeType(), 2);
        }

        QCOMPARE(searchCollection(KMime::Message::mimeType(), 1), QVector<qint64>{});
        QCOMPARE(searchCollection(KMime::Message::mimeType(), 3), QVector<qint64>{ 1 });
    }

    void testCollectionRemove()
    {
        QCOMPARE(searchCollection(KMime::Message::mimeType(), 3), QVector<qint64>{ 1 });

        {
            IndexingPlugin plugin;
            plugin.removeCollection(KMime::Message::mimeType(), 3);
        }

        QCOMPARE(searchCollection(KMime::Message::mimeType(), 3), QVector<qint64>{});
    }
};


QTEST_MAIN(IndexingPluginTest)

#include "indexingplugintest.moc"
