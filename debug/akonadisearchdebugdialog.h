/*
  SPDX-FileCopyrightText: 2014-2023 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akonadisearchdebugsearchpathcombobox.h"
#include "search_debug_export.h"
#include <Akonadi/Item>
#include <QDialog>

namespace Akonadi
{
namespace Search
{
class AkonadiSearchDebugDialogPrivate;
/**
 * @brief The AkonadiSearchDebugDialog class
 * @author Laurent Montel <montel@kde.org>
 */
class AKONADI_SEARCH_DEBUG_EXPORT AkonadiSearchDebugDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AkonadiSearchDebugDialog(QWidget *parent = nullptr);
    ~AkonadiSearchDebugDialog() override;

    void setAkonadiId(Akonadi::Item::Id akonadiId);
    void setSearchType(AkonadiSearchDebugSearchPathComboBox::SearchType type);
    void doSearch();
private Q_SLOTS:
    AKONADI_SEARCH_DEBUG_NO_EXPORT void slotSaveAs();

private:
    AKONADI_SEARCH_DEBUG_NO_EXPORT void readConfig();
    AKONADI_SEARCH_DEBUG_NO_EXPORT void writeConfig();
    AKONADI_SEARCH_DEBUG_NO_EXPORT void saveTextAs(const QString &text, const QString &filter);
    [[nodiscard]] bool saveToFile(const QString &filename, const QString &text);
    AkonadiSearchDebugDialogPrivate *const d;
};
}
}
