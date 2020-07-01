/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */
#include <scheduler.h>
#include <collectionindexingjob.h>

#include <QTest>
#include <AkonadiCore/Collection>
#include <AkonadiCore/ServerManager>
#include <AkonadiCore/qtest_akonadi.h>
#include <KConfig>
#include <KConfigGroup>
#include <QStandardPaths>

class DummyIndexingJob : public CollectionIndexingJob
{
    Q_OBJECT
public:
    DummyIndexingJob(Index &index, const Akonadi::Collection &col, const QList<Akonadi::Item::Id> &pending, QObject *parent = nullptr)
        : CollectionIndexingJob(index, col, pending, parent)
    {
    }

    void start() override
    {
        QMetaObject::invokeMethod(this, &DummyIndexingJob::finish, Qt::QueuedConnection);
    }

private Q_SLOTS:
    void finish()
    {
        emitResult();
    }
};

class DummyJobFactory : public JobFactory
{
public:
    Akonadi::Collection::List indexedCollections;
    Akonadi::Item::List indexedItems;
    QList<bool> fullSyncs;

    CollectionIndexingJob *createCollectionIndexingJob(Index &index, const Akonadi::Collection &col, const QList<Akonadi::Item::Id> &pending, bool fullSync, QObject *parent = nullptr) override
    {
        for (qint64 id : pending) {
            indexedItems << Akonadi::Item(id);
        }
        indexedCollections << col;
        fullSyncs << fullSync;
        return new DummyIndexingJob(index, col, pending, parent);
    }
};

class SchedulerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        qRegisterMetaType<Akonadi::Collection::Id>("Akonadi::Collection::Id");
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        AkonadiTest::checkTestIsIsolated();
        AkonadiTest::setAllResourcesOffline();
        Akonadi::AgentInstance agent = Akonadi::AgentManager::self()->instance(QStringLiteral("akonadi_knut_resource_0"));
        QVERIFY(agent.isValid());
        agent.setIsOnline(true);
        QTRY_VERIFY(agent.isOnline());
    }

    void testInitialIndexing()
    {
        auto config = KSharedConfig::openConfig(QStringLiteral("akonadi_indexing_agent"));
        Index index;
        QSharedPointer<DummyJobFactory> factory(new DummyJobFactory());
        Scheduler scheduler(index, config, factory);
        QSignalSpy statusSpy(&scheduler, &Scheduler::status);
        scheduler.setBusyTimeout(0);
        //Wait for ready signal (indicates that indexing is complete)
        QTRY_COMPARE(statusSpy.count(), 1);
        QTRY_COMPARE(factory->indexedCollections.size(), 2);
        QVERIFY(factory->fullSyncs.at(0));
        QVERIFY(factory->fullSyncs.at(1));
    }

    void testIndexCollections()
    {
        auto config = KSharedConfig::openConfig(QStringLiteral("akonadi_indexing_agent"));
        KConfigGroup group = config->group("General");
        group.writeEntry("initialIndexingComplete", true);

        Index index;
        QSharedPointer<DummyJobFactory> factory(new DummyJobFactory());
        Scheduler scheduler(index, config, factory);
        QSignalSpy statusSpy(&scheduler, &Scheduler::status);
        QSignalSpy finishedIndexing(&scheduler, &Scheduler::collectionIndexingFinished);
        scheduler.setBusyTimeout(0);

        Akonadi::Collection col1(3);
        scheduler.scheduleCollection(col1);
        Akonadi::Collection col2(4);
        scheduler.scheduleCollection(col2, true);

        //Wait for ready signal (indicates that indexing is complete)
        QTRY_COMPARE(statusSpy.count(), 1);
        QTRY_COMPARE(factory->indexedCollections.size(), 2);
        QCOMPARE(factory->indexedCollections.at(0).id(), col1.id());
        QVERIFY(!factory->fullSyncs.at(0));
        QCOMPARE(factory->indexedCollections.at(1).id(), col2.id());
        QVERIFY(factory->fullSyncs.at(1));
        //We index 2 collections.
        QCOMPARE(finishedIndexing.count(), 2);
    }

    void testIndexItems()
    {
        auto config = KSharedConfig::openConfig(QStringLiteral("akonadi_indexing_agent"));
        KConfigGroup group = config->group("General");
        group.writeEntry("initialIndexingComplete", true);

        Index index;
        QSharedPointer<DummyJobFactory> factory(new DummyJobFactory());
        Scheduler scheduler(index, config, factory);
        QSignalSpy statusSpy(&scheduler, &Scheduler::status);
        scheduler.setBusyTimeout(0);

        Akonadi::Collection parent1(3);
        Akonadi::Item item1(1);
        item1.setParentCollection(parent1);
        scheduler.addItem(item1);

        Akonadi::Item item2(2);
        item2.setParentCollection(parent1);
        scheduler.addItem(item2);

        Akonadi::Collection parent2(4);
        Akonadi::Item item3(3);
        item3.setParentCollection(parent2);
        scheduler.addItem(item3);

        //Wait for ready signal (indicates that indexing is complete)
        QTRY_COMPARE(statusSpy.count(), 1);
        QTRY_COMPARE(factory->indexedCollections.size(), 2);
        QCOMPARE(factory->indexedCollections.at(0).id(), parent1.id());
        QVERIFY(!factory->fullSyncs.at(0));
        QCOMPARE(factory->indexedCollections.at(1).id(), parent2.id());
        QVERIFY(!factory->fullSyncs.at(1));
        QCOMPARE(factory->indexedItems.size(), 3);
        QCOMPARE(factory->indexedItems.at(0).id(), item1.id());
        QCOMPARE(factory->indexedItems.at(1).id(), item2.id());
        QCOMPARE(factory->indexedItems.at(2).id(), item3.id());
    }

    void testDirtyCollections()
    {
        auto config = KSharedConfig::openConfig(QStringLiteral("akonadi_indexing_agent"));
        KConfigGroup group = config->group("General");
        group.writeEntry("initialIndexingComplete", true);
        Akonadi::Collection col1(1);

        Index index;

        //Populate dirty collections
        {
            QSharedPointer<DummyJobFactory> factory(new DummyJobFactory());
            Scheduler scheduler(index, config, factory);
            scheduler.scheduleCollection(col1, true);
        }

        QSharedPointer<DummyJobFactory> factory(new DummyJobFactory());
        Scheduler scheduler(index, config, factory);
        QSignalSpy statusSpy(&scheduler, &Scheduler::status);
        QSignalSpy finishedIndexing(&scheduler, &Scheduler::collectionIndexingFinished);

        scheduler.setBusyTimeout(0);

        QTRY_COMPARE(statusSpy.count(), 1);
        QCOMPARE(factory->indexedCollections.size(), 1);
        QCOMPARE(factory->indexedCollections.at(0).id(), col1.id());
        QVERIFY(factory->fullSyncs.at(0));

        QCOMPARE(finishedIndexing.count(), 1);
    }
};

QTEST_MAIN(SchedulerTest)

#include "schedulertest.moc"
