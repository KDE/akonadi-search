/*
  SPDX-FileCopyrightText: 2014-2026 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akonadisearchdebugsearchpathcombobox.h"
#include "search_debug_export.h"
#include <Akonadi/Item>
#include <QWidget>
class QLineEdit;
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
    ~AkonadiSearchDebugWidget() override;

    void setAkonadiId(Akonadi::Item::Id akonadiId);
    void setSearchType(AkonadiSearchDebugSearchPathComboBox::SearchType type);
    void doSearch();

    [[nodiscard]] QString plainText() const;

private:
    AKONADI_SEARCH_DEBUG_NO_EXPORT void slotSearchLineTextChanged(const QString &text);
    AKONADI_SEARCH_DEBUG_NO_EXPORT void slotSearch();
    AKONADI_SEARCH_DEBUG_NO_EXPORT void slotResult(const QString &result);
    AKONADI_SEARCH_DEBUG_NO_EXPORT void slotError(const QString &errorStr);
    QPlainTextEdit *const mPlainTextEditor;
    AkonadiSearchDebugSearchPathComboBox *const mSearchPathComboBox;
    QLineEdit *const mLineEdit;
    QPushButton *const mSearchButton;
};
}
}
