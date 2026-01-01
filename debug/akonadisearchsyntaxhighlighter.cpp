/*
  SPDX-FileCopyrightText: 2014-2026 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akonadisearchsyntaxhighlighter.h"
using namespace Qt::Literals::StringLiterals;

using namespace Akonadi::Search;

AkonadiSearchSyntaxHighlighter::AkonadiSearchSyntaxHighlighter(QTextDocument *doc)
    : QSyntaxHighlighter(doc)
{
    init();
}

AkonadiSearchSyntaxHighlighter::~AkonadiSearchSyntaxHighlighter() = default;

void AkonadiSearchSyntaxHighlighter::highlightBlock(const QString &text)
{
    for (const Rule &rule : std::as_const(m_rules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatchView(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
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
    testType << u"C\\d+"_s;

    // Emails:
    // From
    testType << u"\\bF"_s;
    // To
    testType << u"\\bT"_s;
    // CC
    testType << u"\\bCC"_s;
    // BC
    testType << u"\\bBC"_s;
    // Organization
    testType << u"\\bO"_s;
    // Reply To
    testType << u"\\bRT"_s;
    // Resent-from
    testType << u"\\bRF"_s;
    // List Id
    testType << u"\\bLI"_s;
    // X-Loop
    testType << u"\\bXL"_s;
    // X-Mailing-List
    testType << u"\\bXML"_s;
    // X-Spam-Flag
    testType << u"\\bXSF"_s;
    // BO body element
    testType << u"\\bBO"_s;

    // Contacts:
    // Name
    testType << u"\\bNA"_s;
    // NickName
    testType << u"\\bNI"_s;

    // Calendar
    testType << u"\\bO"_s;
    testType << u"\\bPS"_s;
    testType << u"\\bS"_s;
    testType << u"\\bL"_s;
    for (const QString &s : std::as_const(testType)) {
        const QRegularExpression regex(s);
        m_rules.append(Rule(regex, testFormat));
    }
}

#include "moc_akonadisearchsyntaxhighlighter.cpp"
