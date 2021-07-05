/*
  SPDX-FileCopyrightText: 2014-2021 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akonadisearchsyntaxhighlighter.h"
#include <QRegExp>

using namespace Akonadi::Search;

AkonadiSearchSyntaxHighlighter::AkonadiSearchSyntaxHighlighter(QTextDocument *doc)
    : QSyntaxHighlighter(doc)
{
    init();
}

AkonadiSearchSyntaxHighlighter::~AkonadiSearchSyntaxHighlighter()
{
}

void AkonadiSearchSyntaxHighlighter::highlightBlock(const QString &text)
{
    for (const Rule &rule : std::as_const(m_rules)) {
        const QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        int length = 0;
        while (index >= 0 && (length = expression.matchedLength()) > 0) {
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
}

void AkonadiSearchSyntaxHighlighter::init()
{
    QTextCharFormat testFormat;
    testFormat.setForeground(Qt::black);
    testFormat.setFontWeight(QFont::Bold);
    QStringList testType;
    // Collection
    testType << QStringLiteral("C\\d+");

    // Emails:
    // From
    testType << QStringLiteral("\\bF");
    // To
    testType << QStringLiteral("\\bT");
    // CC
    testType << QStringLiteral("\\bCC");
    // BC
    testType << QStringLiteral("\\bBC");
    // Organization
    testType << QStringLiteral("\\bO");
    // Reply To
    testType << QStringLiteral("\\bRT");
    // Resent-from
    testType << QStringLiteral("\\bRF");
    // List Id
    testType << QStringLiteral("\\bLI");
    // X-Loop
    testType << QStringLiteral("\\bXL");
    // X-Mailing-List
    testType << QStringLiteral("\\bXML");
    // X-Spam-Flag
    testType << QStringLiteral("\\bXSF");
    // BO body element
    testType << QStringLiteral("\\bBO");

    // Contacts:
    // Name
    testType << QStringLiteral("\\bNA");
    // NickName
    testType << QStringLiteral("\\bNI");

    // Calendar
    testType << QStringLiteral("\\bO");
    testType << QStringLiteral("\\bPS");
    testType << QStringLiteral("\\bS");
    testType << QStringLiteral("\\bL");
    for (const QString &s : std::as_const(testType)) {
        const QRegExp regex(s, Qt::CaseSensitive);
        m_rules.append(Rule(regex, testFormat));
    }
}
