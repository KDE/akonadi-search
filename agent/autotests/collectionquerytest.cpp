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

#include "../lib/collectionquery.h"
#include "../lib/resultiterator.h"
#include "index.h"
#include "query.h"
#include <QDebug>

Q_DECLARE_METATYPE(QSet<qint64>)
Q_DECLARE_METATYPE(QList<qint64>)

class CollectionQueryTest : public QObject
{
    Q_OBJECT
private:
    QString collectionsDir;

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

    QString dbPrefix;
    Index index;

private Q_SLOTS:
    void init()
    {
        dbPrefix = QDir::tempPath() + "/collectiontest"_L1;
        index.setOverrideDbPrefixPath(dbPrefix);
        index.createIndexers();
    }

    void cleanup()
    {
        removeDir(dbPrefix);
    }

    void searchByName()
    {
        Akonadi::Collection col1(3);
        col1.setName(u"col3"_s);
        index.index(col1);

        Akonadi::Collection col2(4);
        col2.setName(u"col4"_s);
        col2.setParentCollection(col1);
        index.index(col2);

        Akonadi::Search::PIM::CollectionQuery query;
        query.setDatabaseDir(dbPrefix + u"/collections/"_s);
        query.nameMatches(col1.name());
        Akonadi::Search::PIM::ResultIterator it = query.exec();
        QList<qint64> results;
        while (it.next()) {
            // qDebug() << "result " << it.id();
            results << it.id();
        }
        QCOMPARE(results.size(), 1);
        QCOMPARE(results.at(0), col1.id());
    }

    void searchHierarchy()
    {
        Akonadi::Collection col1(1);
        col1.setName(u"col1"_s);
        index.index(col1);

        Akonadi::Collection col2(2);
        col2.setName(u"col2"_s);
        col2.setParentCollection(col1);
        index.index(col2);

        {
            Akonadi::Search::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix + "/collections/"_L1);
            query.pathMatches(col1.name());
            Akonadi::Search::PIM::ResultIterator it = query.exec();
            QList<qint64> results;
            while (it.next()) {
                // qDebug() << "result " << it.id();
                results << it.id();
            }
            std::sort(results.begin(), results.end());
            QCOMPARE(results.size(), 2);
            QCOMPARE(results.at(0), col1.id());
            QCOMPARE(results.at(1), col2.id());
        }
        {
            Akonadi::Search::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix + "/collections/"_L1);
            query.pathMatches(u"/col1/col2"_s);
            Akonadi::Search::PIM::ResultIterator it = query.exec();
            QList<qint64> results;
            while (it.next()) {
                // qDebug() << "result " << it.id();
                results << it.id();
            }
            std::sort(results.begin(), results.end());
            QCOMPARE(results.size(), 1);
            QCOMPARE(results.at(0), col2.id());
        }
    }

    QList<qint64> getResults(Akonadi::Search::PIM::ResultIterator it)
    {
        QList<qint64> results;
        while (it.next()) {
            // qDebug() << "result " << it.id();
            results << it.id();
        }
        return results;
    }

    void collectionChanged()
    {
        Akonadi::Collection col1(1);
        col1.setName(u"col1"_s);
        index.index(col1);

        Akonadi::Collection col2(2);
        col2.setName(u"col2"_s);
        col2.setParentCollection(col1);
        index.index(col2);

        Akonadi::Collection col3(3);
        col3.setName(u"col3"_s);
        col3.setParentCollection(col2);
        index.index(col3);

        col1.setName(u"colX"_s);
        index.change(col1);
        col2.setParentCollection(col1);
        index.change(col2);
        col3.setParentCollection(col2);
        index.change(col3);

        // Old name gone
        {
            Akonadi::Search::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix + "/collections/"_L1);
            query.nameMatches(u"col1"_s);
            QList<qint64> results = getResults(query.exec());
            QCOMPARE(results.size(), 0);
        }
        // New name
        {
            Akonadi::Search::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix + "/collections/"_L1);
            query.nameMatches(u"colX"_s);
            QList<qint64> results = getResults(query.exec());
            QCOMPARE(results.size(), 1);
            QCOMPARE(results.at(0), col1.id());
        }
        // Old path gone
        {
            Akonadi::Search::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix + "/collections/"_L1);
            query.pathMatches(u"/col1/col2"_s);
            QList<qint64> results = getResults(query.exec());
            QCOMPARE(results.size(), 0);
        }
        // New paths
        {
            Akonadi::Search::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix + "/collections/"_L1);
            query.pathMatches(u"/colX/col2"_s);
            QList<qint64> results = getResults(query.exec());
            QCOMPARE(results.size(), 2);
            QCOMPARE(results.at(0), col2.id());
            QCOMPARE(results.at(1), col3.id());
        }
        {
            Akonadi::Search::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix + "/collections/"_L1);
            query.pathMatches(u"/colX/col2/col3"_s);
            QList<qint64> results = getResults(query.exec());
            QCOMPARE(results.size(), 1);
            QCOMPARE(results.at(0), col3.id());
        }
    }
};

QTEST_GUILESS_MAIN(CollectionQueryTest)

#include "collectionquerytest.moc"
