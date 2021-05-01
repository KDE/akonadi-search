/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "emailindexer.h"

#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QTimer>

#include <Collection>
#include <CollectionFetchJob>
#include <Item>
#include <ItemFetchJob>
#include <ItemFetchScope>

class App : public QApplication
{
    Q_OBJECT
public:
    App(int &argc, char **argv, int flags = ApplicationFlags);

private Q_SLOTS:
    void main();

    void slotRootCollectionsFetched(KJob *job);
    void indexNextCollection();
    void itemReceived(const Akonadi::Item::List &item);
    void slotIndexed();
    void slotCommitTimerElapsed();

private:
    Akonadi::Collection::List m_collections;
    EmailIndexer m_indexer;

    QElapsedTimer m_totalTime;
    int m_indexTime;
    int m_numEmails;

    QTimer m_commitTimer;
};

int main(int argc, char **argv)
{
    App app(argc, argv);
    return app.exec();
}

App::App(int &argc, char **argv, int flags)
    : QApplication(argc, argv, flags)
    , m_indexer(QStringLiteral("/tmp/xap"), QStringLiteral("/tmp/xapC"))
{
    QTimer::singleShot(0, this, &App::main);
}

void App::main()
{
    m_commitTimer.setInterval(1000);
    connect(&m_commitTimer, &QTimer::timeout, this, &App::slotCommitTimerElapsed);
    m_commitTimer.start();

    auto job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive);
    connect(job, &Akonadi::CollectionFetchJob::finished, this, &App::slotRootCollectionsFetched);
    job->start();

    m_numEmails = 0;
    m_indexTime = 0;
    m_totalTime.start();
}

void App::slotRootCollectionsFetched(KJob *kjob)
{
    auto job = qobject_cast<Akonadi::CollectionFetchJob *>(kjob);
    m_collections = job->collections();

    QMutableVectorIterator<Akonadi::Collection> it(m_collections);
    while (it.hasNext()) {
        const Akonadi::Collection &c = it.next();
        const QStringList mimeTypes = c.contentMimeTypes();
        if (!c.contentMimeTypes().contains(QLatin1String("message/rfc822"))) {
            it.remove();
        }
    }

    if (m_collections.size()) {
        indexNextCollection();
    } else {
        qDebug() << "No collections to index";
    }
}

void App::indexNextCollection()
{
    auto fetchJob = new Akonadi::ItemFetchJob(m_collections.takeFirst(), this);
    fetchJob->fetchScope().fetchAllAttributes(true);
    fetchJob->fetchScope().fetchFullPayload(true);
    fetchJob->fetchScope().setFetchModificationTime(false);
    fetchJob->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    fetchJob->setDeliveryOption(Akonadi::ItemFetchJob::EmitItemsIndividually);

    connect(fetchJob, &Akonadi::ItemFetchJob::itemsReceived, this, &App::itemReceived);
    connect(fetchJob, &Akonadi::ItemFetchJob::result, this, &App::slotIndexed);
}

void App::itemReceived(const Akonadi::Item::List &itemList)
{
    QElapsedTimer timer;
    timer.start();

    for (const Akonadi::Item &item : itemList) {
        m_indexer.index(item);
    }

    m_indexTime += timer.elapsed();
    m_numEmails += itemList.size();
}

void App::slotCommitTimerElapsed()
{
    QElapsedTimer timer;
    timer.start();

    m_indexer.commit();
    m_indexTime += timer.elapsed();

    qDebug() << "Emails:" << m_numEmails;
    qDebug() << "Total Time:" << m_totalTime.elapsed() / 1000.0 << " seconds";
    qDebug() << "Index Time:" << m_indexTime / 1000.0 << " seconds";
}

void App::slotIndexed()
{
    if (!m_collections.isEmpty()) {
        QTimer::singleShot(0, this, &App::indexNextCollection);
        return;
    }

    m_indexer.commit();

    qDebug() << "Emails:" << m_numEmails;
    qDebug() << "Total Time:" << m_totalTime.elapsed() / 1000.0 << " seconds";
    qDebug() << "Index Time:" << m_indexTime / 1000.0 << " seconds";

    // Print the io usage
    QFile file(QStringLiteral("/proc/self/io"));
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream fs(&file);
    QString str = fs.readAll();

    qDebug() << "------- IO ---------";
    QTextStream stream(&str);
    while (!stream.atEnd()) {
        QString str = stream.readLine();

        QString rchar(QStringLiteral("rchar: "));
        if (str.startsWith(rchar)) {
            ulong amt = str.midRef(rchar.size()).toULong();
            qDebug() << "Read:" << amt / 1024 << "kb";
        }

        QString wchar(QStringLiteral("wchar: "));
        if (str.startsWith(wchar)) {
            ulong amt = str.midRef(wchar.size()).toULong();
            qDebug() << "Write:" << amt / 1024 << "kb";
        }

        QString read(QStringLiteral("read_bytes: "));
        if (str.startsWith(read)) {
            ulong amt = str.midRef(read.size()).toULong();
            qDebug() << "Actual Reads:" << amt / 1024 << "kb";
        }

        QString write(QStringLiteral("write_bytes: "));
        if (str.startsWith(write)) {
            ulong amt = str.midRef(write.size()).toULong();
            qDebug() << "Actual Writes:" << amt / 1024 << "kb";
        }
    }
    qDebug() << "\nThe actual read/writes may be 0 because of an existing"
             << "cache and /tmp being memory mapped";
    quit();
}

#include "emailtest.moc"
