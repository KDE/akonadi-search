/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */
#include <collectionindexingjob.h>

#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/ServerManager>
#include <AkonadiCore/qtest_akonadi.h>
#include <QTest>

class TestIndex : public Index
{
public:
    using Index::index; // So we don't trigger -Woverloaded-virtual
    using Index::move; // So we don't trigger -Woverloaded-virtual
    QList<Akonadi::Item::Id> itemsIndexed;
    QList<Akonadi::Item::Id> alreadyIndexed;
    QList<Akonadi::Item::Id> itemsRemoved;

    void commit() override
    {
    }

    bool createIndexers() override
    {
        return true;
    }

    void findIndexed(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id) override
    {
        indexed = QSet<Akonadi::Item::Id>(alreadyIndexed.begin(), alreadyIndexed.end());
    }

    void index(const Akonadi::Item &item) override
    {
        itemsIndexed << item.id();
    }

    qlonglong indexedItems(Akonadi::Collection::Id) override
    {
        return alreadyIndexed.size();
    }

    void move(const Akonadi::Item::List & /* items */, const Akonadi::Collection & /* from */, const Akonadi::Collection & /* to */) override
    {
    }

    void remove(const Akonadi::Collection & /* col */) override
    {
    }

    void remove(const QSet<Akonadi::Item::Id> &ids, const QStringList & /* mimeTypes */) override
    {
        itemsRemoved += ids.values();
    }

    void remove(const Akonadi::Item::List & /* items */) override
    {
    }

    void removeDatabase() override
    {
    }

    bool haveIndexerForMimeTypes(const QStringList &) override
    {
        return true;
    }
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

        auto fetchJob = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive);
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
        auto job = new CollectionIndexingJob(index, itemCollection, QList<Akonadi::Item::Id>());
        job->setFullSync(true);
        AKVERIFYEXEC(job);
        QCOMPARE(index.itemsIndexed.size(), 3);
    }

    void testNoFullSync()
    {
        TestIndex index;
        auto job = new CollectionIndexingJob(index, itemCollection, QList<Akonadi::Item::Id>());
        job->setFullSync(false);
        AKVERIFYEXEC(job);
        QCOMPARE(index.itemsIndexed.size(), 0);
    }

    void testNoFullSyncWithPending()
    {
        TestIndex index;
        auto job = new CollectionIndexingJob(index, itemCollection, QList<Akonadi::Item::Id>() << 1);
        job->setFullSync(false);
        AKVERIFYEXEC(job);
        QCOMPARE(index.itemsIndexed.size(), 1);
    }

    void testFullSyncButUpToDate()
    {
        TestIndex index;
        index.alreadyIndexed << 1 << 2 << 3;
        auto job = new CollectionIndexingJob(index, itemCollection, QList<Akonadi::Item::Id>());
        job->setFullSync(true);
        AKVERIFYEXEC(job);
        QCOMPARE(index.itemsIndexed.size(), 0);
    }

    void testFullSyncWithRemove()
    {
        TestIndex index;
        index.alreadyIndexed << 15 << 16 << 17;
        auto job = new CollectionIndexingJob(index, itemCollection, QList<Akonadi::Item::Id>());
        job->setFullSync(true);
        AKVERIFYEXEC(job);
        QCOMPARE(index.itemsIndexed.size(), 3);
        QCOMPARE(index.itemsRemoved.size(), 3);
    }
};

QTEST_MAIN(CollectionIndexingJobTest)

#include "collectionindexingjobtest.moc"
