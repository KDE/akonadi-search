/*
  SPDX-FileCopyrightText: 2014-2021 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AKONADISEARCHSYNTAXHIGHLIGHTER_H
#define AKONADISEARCHSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

namespace Akonadi {
namespace Search {
class Rule
{
public:
    Rule()
    {
    }

    Rule(const QRegExp &r, const QTextCharFormat &f)
        : pattern(r)
        , format(f)
    {
    }

    QRegExp pattern;
    QTextCharFormat format;
};

class AkonadiSearchSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit AkonadiSearchSyntaxHighlighter(QTextDocument *doc);
    ~AkonadiSearchSyntaxHighlighter() override;

    void highlightBlock(const QString &text) override;

protected:
    void init();
    QVector<Rule> m_rules;
};
}
}
#endif // AKONADISEARCHSYNTAXHIGHLIGHTER_H
