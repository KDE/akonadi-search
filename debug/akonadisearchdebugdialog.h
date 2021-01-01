/*
  SPDX-FileCopyrightText: 2014-2021 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AKONADISEARCHDEBUGDIALOG_H
#define AKONADISEARCHDEBUGDIALOG_H

#include <QDialog>
#include "search_debug_export.h"
#include "akonadisearchdebugsearchpathcombobox.h"
#include <AkonadiCore/Item>

namespace Akonadi {
namespace Search {
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
    ~AkonadiSearchDebugDialog();

    void setAkonadiId(Akonadi::Item::Id akonadiId);
    void setSearchType(AkonadiSearchDebugSearchPathComboBox::SearchType type);
    void doSearch();
private Q_SLOTS:
    void slotSaveAs();
private:
    void readConfig();
    void writeConfig();
    void saveTextAs(const QString &text, const QString &filter);
    AkonadiSearchDebugDialogPrivate *const d;
    bool saveToFile(const QString &filename, const QString &text);
};
}
}
#endif // AKONADISEARCHDEBUGDIALOG_H
