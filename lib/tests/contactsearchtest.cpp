/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "../resultiterator.h"
#include "contactquery.h"

#include <QApplication>
#include <QDebug>
#include <QTimer>

#include <Akonadi/Contact/ContactSearchJob>

using namespace Akonadi::Search::PIM;

class App : public QApplication
{
    Q_OBJECT
public:
    App(int &argc, char **argv, int flags = ApplicationFlags);

private Q_SLOTS:
    void main();
    void slotItemsReceived(const Akonadi::Item::List &list);

private:
};

int main(int argc, char **argv)
{
    App app(argc, argv);
    return app.exec();
}

App::App(int &argc, char **argv, int flags)
    : QApplication(argc, argv, flags)
{
    QTimer::singleShot(0, this, SLOT(main()));
}

void App::main()
{
#if 0
    Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob();
    job->setQuery(Akonadi::ContactSearchJob::NameOrEmail, "to", Akonadi::ContactSearchJob::StartsWithMatch);

    connect(job, SIGNAL(itemsReceived(Akonadi::Item::List)),
            this, SLOT(slotItemsReceived(Akonadi::Item::List)));
    connect(job, SIGNAL(finished(KJob*)),
            this, SLOT(quit()));
    job->start();
    qDebug() << "Query started";
#endif

    ContactQuery q;
    q.matchEmail(QLatin1String("t"));
    q.setMatchCriteria(ContactQuery::StartsWithMatch);

    ResultIterator iter = q.exec();
    while (iter.next()) {
        qDebug() << iter.id();
    }
}

void App::slotItemsReceived(const Akonadi::Item::List &list)
{
    qDebug() << list.size();
    for (const Akonadi::Item &item : list) {
        qDebug() << item.id() << item.mimeType();
    }
}

#include "contactsearchtest.moc"
