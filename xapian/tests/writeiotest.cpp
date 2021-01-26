/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QUuid>

#include "xapiandatabase.h"
#include "xapiandocument.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("num"), QStringLiteral("The number of terms. Each term is of length 10"));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("p") << QStringLiteral("position"), QStringLiteral("Add positional information")));
    parser.addHelpOption();
    parser.process(app);

    QStringList args = parser.positionalArguments();
    if (args.size() != 1) {
        parser.showHelp(1);
    }

    QTemporaryDir tempDir;
    tempDir.setAutoRemove(false);

    Akonadi::Search::XapianDatabase db(tempDir.path(), true);

    qDebug() << tempDir.path();
    qDebug() << "Creating the document";

    Akonadi::Search::XapianDocument doc;
    int size = args.first().toInt();

    for (int i = 0; i < size; i++) {
        QByteArray term = QUuid::createUuid().toByteArray().mid(1, 10);

        if (parser.isSet(QStringLiteral("p"))) {
            std::string stdString(term.constData(), term.length());
            doc.doc().add_posting(stdString, i);
        } else {
            doc.addTerm(QString::fromUtf8(term));
        }
    }

    db.replaceDocument(1, doc);
    db.commit();

    int dbSize = 0;
    QDir dbDir(tempDir.path());
    const auto entryInfoList = dbDir.entryInfoList(QDir::Files);
    for (const QFileInfo &file : entryInfoList) {
        qDebug() << file.fileName() << file.size() / 1024 << "kb";
        dbSize += file.size();
    }
    qDebug() << "Database Size:" << dbSize / 1024 << "kb";

    return app.exec();
}
