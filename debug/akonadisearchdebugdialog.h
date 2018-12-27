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

#ifndef AKONADISEARCHDEBUGDIALOG_H
#define AKONADISEARCHDEBUGDIALOG_H

#include <QDialog>
#include "search_debug_export.h"
#include "akonadisearchdebugsearchpathcombobox.h"
#include <AkonadiCore/Item>

namespace Akonadi
{
namespace Search
{
class AkonadiSearchDebugDialogPrivate;
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

