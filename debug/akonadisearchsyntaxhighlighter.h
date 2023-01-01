/*
  SPDX-FileCopyrightText: 2014-2023 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include <QSyntaxHighlighter>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QRegExp>
#else
#include <QRegularExpression>
#endif
namespace Akonadi
{
namespace Search
{
class Rule
{
public:
    Rule() = default;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Rule(const QRegExp &r, const QTextCharFormat &f)
#else
    Rule(const QRegularExpression &r, const QTextCharFormat &f)
#endif
        : pattern(r)
        , format(f)
    {
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QRegExp pattern;
#else
    QRegularExpression pattern;
#endif
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
