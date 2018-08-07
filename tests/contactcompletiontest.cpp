/*
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
 * Copyright (C) 2018  Daniel Vrátil <dvratil@kde.org>
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

#include "contactcompleter.h"

#include <QCoreApplication>
#include <QTimer>
#include <QElapsedTimer>

#include <iostream>

using namespace Akonadi::Search;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " QUERY" << std::endl;
        exit(1);
    }

    const auto query = QString::fromUtf8(argv[1]);

    QElapsedTimer timer;
    ContactCompleter completer(query, 100);
    completer.setAutoDelete(false);
    QObject::connect(&completer, &ContactCompleter::finished,
                     &app, [&app, &timer](const QStringList &results) {
                         for (const auto &result : results) {
                             std::cout << result.toStdString() << std::endl;
                         }
                         std::cout << "Fetched " << results.count() << " results in " << timer.elapsed() << "ms" << std::endl;
                         QTimer::singleShot(0, &app, &QCoreApplication::quit);
                     });
    timer.start();
    completer.start();

    return app.exec();
}
