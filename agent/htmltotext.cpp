// SPDX-FileCopyrightText: 2023 g10 code GmbH
// SPDX-Contributor: Carl Schwan <carl.schwan@gnupg.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include <QGuiApplication>

#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <iostream>

#ifdef Q_OS_WIN32
#include <fcntl.h>
#include <io.h>
#endif

#ifdef HAS_HTMLPARSER
#include <lib.rs.h>
#else
#include <QTextDocument>
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("akonadi_search"));

    KAboutData about(QStringLiteral("akonadi_html_to_text"),
                     i18n("Akonadi HTML To Text"),
                     QStringLiteral("1.0"),
                     i18n("Akonadi HTML To Text Converter"),
                     KAboutLicense::LGPL_V2,
                     i18n("© 2023-2024 KDE Community"));

    about.addAuthor(i18n("Carl Schwan"), i18n("Maintainer"), QStringLiteral("carl@carlschwan.eu"), QStringLiteral("https://carlschwan.eu"));

    KAboutData::setApplicationData(about);

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Akonadi HTML To Text Converter"));

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    QByteArray content;

#ifdef Q_OS_WIN32
    _setmode(_fileno(stdin), _O_BINARY);
#endif

    while (!std::cin.eof()) {
        char arr[1024];
        std::cin.read(arr, sizeof(arr));
        int s = std::cin.gcount();
        content.append(arr, s);
    }

#ifdef HAS_HTMLPARSER
    const auto html = content.toStdString();
    const auto text = std::string(convert_to_text(rust::String(html)));
#else
    QTextDocument doc;
    doc.setHtml(QString::fromUtf8(content));
    const std::string text(doc.toPlainText().toStdString());
#endif

    std::cout << text;

    return 0;
}
