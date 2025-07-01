/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include <QCommandLineOption>
using namespace Qt::Literals::StringLiterals;

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QUuid>

#include "xapiandocument.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument(u"num"_s, QStringLiteral("The number of terms. Each term is of length 10"));
    parser.addOption(QCommandLineOption(QStringList() << u"p"_s << QStringLiteral("position"), QStringLiteral("Add positional information")));
    parser.addHelpOption();
    parser.process(app);

    QStringList args = parser.positionalArguments();
    if (args.size() != 1) {
        parser.showHelp(1);
    }

    Akonadi::Search::XapianDocument doc;
    const int size = args.first().toInt();

    for (int i = 0; i < size; i++) {
        QByteArray term = QUuid::createUuid().toByteArray().mid(1, 10);

        if (parser.isSet(u"p"_s)) {
            const std::string stdString(term.constData(), term.length());
            doc.doc().add_posting(stdString, i);
        } else {
            doc.addTerm(QString::fromUtf8(term));
        }
    }

    qDebug() << "Added" << size << "terms";
    if (parser.isSet(u"p"_s)) {
        qDebug() << "With Positional Information";
    }
    return app.exec();
}
