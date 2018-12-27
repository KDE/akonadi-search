/*
  Copyright (c) 2014-2019 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
    for (const Rule &rule : qAsConst(m_rules)) {
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
    //Collection
    testType << QStringLiteral("C\\d+");

    //Emails:
    //From
    testType << QStringLiteral("\\bF");
    //To
    testType << QStringLiteral("\\bT");
    //CC
    testType << QStringLiteral("\\bCC");
    //BC
    testType << QStringLiteral("\\bBC");
    //Organization
    testType << QStringLiteral("\\bO");
    //Reply To
    testType << QStringLiteral("\\bRT");
    //Resent-from
    testType << QStringLiteral("\\bRF");
    //List Id
    testType << QStringLiteral("\\bLI");
    //X-Loop
    testType << QStringLiteral("\\bXL");
    //X-Mailing-List
    testType << QStringLiteral("\\bXML");
    //X-Spam-Flag
    testType << QStringLiteral("\\bXSF");
    //BO body element
    testType << QStringLiteral("\\bBO");

    //Contacts:
    //Name
    testType << QStringLiteral("\\bNA");
    //NickName
    testType << QStringLiteral("\\bNI");

    //Calendar
    testType << QStringLiteral("\\bO");
    testType << QStringLiteral("\\bPS");
    testType << QStringLiteral("\\bS");
    testType << QStringLiteral("\\bL");
    for (const QString &s : qAsConst(testType)) {
        const QRegExp regex(s, Qt::CaseSensitive);
        m_rules.append(Rule(regex, testFormat));
    }
}

