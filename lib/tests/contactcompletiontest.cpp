/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "../contactcompleter.h"

#include <iostream>
#include <QCoreApplication>
#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>

#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>

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

App::App(int &argc, char **argv, int flags) : QCoreApplication(argc, argv, flags)
{
    QTimer::singleShot(0, this, &App::main);
}

void App::main()
{
    ContactCompleter com(m_query, 100);

    QElapsedTimer timer;
    timer.start();

    const QStringList emails = com.complete();
    for (const QString &em : qAsConst(emails)) {
        std::cout << em.toUtf8().data() << std::endl;
    }

    qDebug() << timer.elapsed();
    quit();
}

#include "contactcompletiontest.moc"
