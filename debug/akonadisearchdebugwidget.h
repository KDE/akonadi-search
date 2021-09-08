/*
  SPDX-FileCopyrightText: 2014-2021 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akonadisearchdebugsearchpathcombobox.h"
#include "search_debug_export.h"
#include <Akonadi/Item>
#include <QWidget>
class KLineEdit;
class QPushButton;
class QPlainTextEdit;
namespace Akonadi
{
namespace Search
{
/**
 * @brief The AkonadiSearchDebugWidget class
 */
class AKONADI_SEARCH_DEBUG_EXPORT AkonadiSearchDebugWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AkonadiSearchDebugWidget(QWidget *parent = nullptr);
    ~AkonadiSearchDebugWidget();

    void setAkonadiId(Akonadi::Item::Id akonadiId);
    void setSearchType(AkonadiSearchDebugSearchPathComboBox::SearchType type);
    void doSearch();

    QString plainText() const;

private Q_SLOTS:
    void slotSearchLineTextChanged(const QString &text);
    void slotSearch();
    void slotResult(const QString &result);
    void slotError(const QString &errorStr);

private:
    QPlainTextEdit *mPlainTextEditor = nullptr;
    AkonadiSearchDebugSearchPathComboBox *mSearchPathComboBox = nullptr;
    KLineEdit *mLineEdit = nullptr;
    QPushButton *mSearchButton = nullptr;
};
}
}
