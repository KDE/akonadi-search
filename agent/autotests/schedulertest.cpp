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
#include <scheduler.h>
#include <collectionindexingjob.h>

#include <QTest>
#include <AkonadiCore/Collection>
#include <AkonadiCore/ServerManager>
#include <AkonadiCore/qtest_akonadi.h>
#include <KConfig>
#include <KConfigGroup>

class DummyIndexingJob : public CollectionIndexingJob
{
    Q_OBJECT
public:
    DummyIndexingJob(Index &index, const Akonadi::Collection &col,
                     const QList<Akonadi::Item::Id> &pending, QObject *parent = nullptr)
        : CollectionIndexingJob(index, col, pending, parent)
    {
    }

    virtual void start()
    {
        QMetaObject::invokeMethod(this, "finish", Qt::QueuedConnection);
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

    virtual CollectionIndexingJob *createCollectionIndexingJob(Index &index,
                                                               const Akonadi::Collection &col,
                                                               const QList<Akonadi::Item::Id> &pending,
                                                               bool fullSync, QObject *parent = nullptr)
    {
        Q_FOREACH (qint64 id, pending) {
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
        QSignalSpy statusSpy(&scheduler, SIGNAL(status(int,QString)));
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
        QSignalSpy statusSpy(&scheduler, SIGNAL(status(int,QString)));
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
    }

    void testIndexItems()
    {
        auto config = KSharedConfig::openConfig(QStringLiteral("akonadi_indexing_agent"));
        KConfigGroup group = config->group("General");
        group.writeEntry("initialIndexingComplete", true);

        Index index;
        QSharedPointer<DummyJobFactory> factory(new DummyJobFactory());
        Scheduler scheduler(index, config, factory);
        QSignalSpy statusSpy(&scheduler, SIGNAL(status(int,QString)));
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
        QSignalSpy statusSpy(&scheduler, SIGNAL(status(int,QString)));
        scheduler.setBusyTimeout(0);

        QTRY_COMPARE(statusSpy.count(), 1);
        QCOMPARE(factory->indexedCollections.size(), 1);
        QCOMPARE(factory->indexedCollections.at(0).id(), col1.id());
        QVERIFY(factory->fullSyncs.at(0));
    }
};

QTEST_MAIN(SchedulerTest)

#include "schedulertest.moc"

