/*
  SPDX-FileCopyrightText: 2014-2026 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include <QRegularExpression>
#include <QSyntaxHighlighter>
namespace Akonadi
{
namespace Search
{
class Rule
{
public:
    Rule() = default;
    Rule(const QRegularExpression &r, const QTextCharFormat &f)
        : pattern(r)
        , format(f)
    {
    }
    QRegularExpression pattern;
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
    QList<Rule> m_rules;
};
}
}
