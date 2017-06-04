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
#include <collectionindexingjob.h>

#include <QTest>
#include <AkonadiCore/Collection>
#include <AkonadiCore/ServerManager>
#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/qtest_akonadi.h>
#include <KConfigGroup>

class TestIndex : public Index
{
public:
    using Index::move; // So we don't trigger -Woverloaded-virtual
    using Index::index; // So we don't trigger -Woverloaded-virtual
    QList<Akonadi::Item::Id> itemsIndexed;
    QList<Akonadi::Item::Id> alreadyIndexed;
    QList<Akonadi::Item::Id> itemsRemoved;

    void commit() override {};
    bool createIndexers() override {
        return true;
    };
    void findIndexed(QSet< Akonadi::Item::Id> &indexed, Akonadi::Collection::Id) override {
        indexed = alreadyIndexed.toSet();
    };
    void index(const Akonadi::Item &item) override {
        itemsIndexed << item.id();
    };
    qlonglong indexedItems(Akonadi::Collection::Id) override {
        return alreadyIndexed.size();
    };
    void move(const Akonadi::Item::List & /* items */,
              const Akonadi::Collection & /* from */,
              const Akonadi::Collection & /* to */) override {};
    void remove(const Akonadi::Collection & /* col */) override {};
    void remove(const QSet<Akonadi::Item::Id> &ids, const QStringList & /* mimeTypes */) override {
        itemsRemoved += ids.toList();
    };
    void remove(const Akonadi::Item::List & /* items */) override {};
    void removeDatabase() override {};
    bool haveIndexerForMimeTypes(const QStringList &) override {
        return true;
    };
};

class CollectionIndexingJobTest : public QObject
{
    Q_OBJECT

private:
    Akonadi::Collection itemCollection;

private Q_SLOTS:

    void init()
    {
        AkonadiTest::checkTestIsIsolated();
        AkonadiTest::setAllResourcesOffline();
        Akonadi::AgentInstance agent = Akonadi::AgentManager::self()->instance(QStringLiteral("akonadi_knut_resource_0"));
        QVERIFY(agent.isValid());
        agent.setIsOnline(true);

        Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive);
        fetchJob->exec();
        const Akonadi::Collection::List lstCols = fetchJob->collections();
        for (const Akonadi::Collection &col : lstCols) {
            if (col.name() == QLatin1String("foo")) {
                itemCollection = col;
            }
        }
        QVERIFY(itemCollection.isValid());
    }

    void testFullSync()
    {
        TestIndex index;
        CollectionIndexingJob *job = new CollectionIndexingJob(index, itemCollection, QList<Akonadi::Item::Id>());
        job->setFullSync(true);
        AKVERIFYEXEC(job);
        QCOMPARE(index.itemsIndexed.size(), 3);
    }

    void testNoFullSync()
    {
        TestIndex index;
        CollectionIndexingJob *job = new CollectionIndexingJob(index, itemCollection, QList<Akonadi::Item::Id>());
        job->setFullSync(false);
        AKVERIFYEXEC(job);
        QCOMPARE(index.itemsIndexed.size(), 0);
    }

    void testNoFullSyncWithPending()
    {
        TestIndex index;
        CollectionIndexingJob *job = new CollectionIndexingJob(index, itemCollection, QList<Akonadi::Item::Id>() << 1);
        job->setFullSync(false);
        AKVERIFYEXEC(job);
        QCOMPARE(index.itemsIndexed.size(), 1);
    }

    void testFullSyncButUpToDate()
    {
        TestIndex index;
        index.alreadyIndexed << 1 << 2 << 3;
        CollectionIndexingJob *job = new CollectionIndexingJob(index, itemCollection, QList<Akonadi::Item::Id>());
        job->setFullSync(true);
        AKVERIFYEXEC(job);
        QCOMPARE(index.itemsIndexed.size(), 0);
    }

    void testFullSyncWithRemove()
    {
        TestIndex index;
        index.alreadyIndexed << 15 << 16 << 17;
        CollectionIndexingJob *job = new CollectionIndexingJob(index, itemCollection, QList<Akonadi::Item::Id>());
        job->setFullSync(true);
        AKVERIFYEXEC(job);
        QCOMPARE(index.itemsIndexed.size(), 3);
        QCOMPARE(index.itemsRemoved.size(), 3);
    }

};

QTEST_MAIN(CollectionIndexingJobTest)

#include "collectionindexingjobtest.moc"
