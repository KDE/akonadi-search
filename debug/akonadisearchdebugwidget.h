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

#ifndef AKONADISEARCHDEBUGWIDGET_H
#define AKONADISEARCHDEBUGWIDGET_H

#include <QWidget>
#include "search_debug_export.h"
#include "akonadisearchdebugsearchpathcombobox.h"
#include <AkonadiCore/Item>
class KLineEdit;
class QPushButton;
class QPlainTextEdit;
namespace Akonadi {
namespace Search {
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
#endif // AKONADISEARCHDEBUGWIDGET_H
