/*
  SPDX-FileCopyrightText: 2014-2025 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "../akonadisearchdebugdialog.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QStandardPaths>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    auto dlg = new Akonadi::Search::AkonadiSearchDebugDialog();
    dlg->resize(800, 600);
    dlg->show();
    app.exec();
    delete dlg;
    return 0;
}
