/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "../emailquery.h"
#include "../resultiterator.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

#include <KMime/Message>

using namespace Akonadi::Search::PIM;

class App : public QCoreApplication
{
    Q_OBJECT
public:
    App(int &argc, char **argv, int flags = ApplicationFlags);

    QString m_query;

private Q_SLOTS:
    void main();
    void itemsReceived(const Akonadi::Item::List &item);
};

int main(int argc, char **argv)
{
    App app(argc, argv);

    if (argc != 2) {
        qWarning() << "Proper args required";
        exit(0);
    }
    app.m_query = QString::fromUtf8(argv[1]);

    return app.exec();
}

App::App(int &argc, char **argv, int flags)
    : QCoreApplication(argc, argv, flags)
{
    QTimer::singleShot(0, this, &App::main);
}

void App::main()
{
    EmailQuery query;
    query.matches(m_query);
    query.setLimit(100);

    QList<Akonadi::Item::Id> m_akonadiIds;

    ResultIterator it = query.exec();
    while (it.next()) {
        m_akonadiIds << it.id();
    }
    qDebug() << "Got" << m_akonadiIds.size() << "items";

    if (m_akonadiIds.isEmpty()) {
        quit();
        return;
    }

    auto job = new Akonadi::ItemFetchJob(m_akonadiIds);
    job->fetchScope().fetchFullPayload(true);

    connect(job, &Akonadi::ItemFetchJob::itemsReceived, this, &App::itemsReceived);
    connect(job, &Akonadi::ItemFetchJob::finished, this, &App::quit);

    job->start();
}

void App::itemsReceived(const Akonadi::Item::List &itemList)
{
    for (const Akonadi::Item &item : itemList) {
        auto message = item.payload<QSharedPointer<KMime::Message>>();
        QDateTime date = message->date()->dateTime();
        qDebug() << date.toString(Qt::ISODate) << message->subject()->asUnicodeString();
    }
}

#include "emailquerytest.moc"
