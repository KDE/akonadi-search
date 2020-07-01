/*
  SPDX-FileCopyrightText: 2014-2020 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AKONADISEARCHDEBUGSEARCHPATHCOMBOBOX_H
#define AKONADISEARCHDEBUGSEARCHPATHCOMBOBOX_H

#include <QComboBox>
#include "search_debug_export.h"
namespace Akonadi {
namespace Search {
/**
 * @brief The AkonadiSearchDebugSearchPathComboBox class
 * @author Laurent Montel <montel@kde.org>
 */
class AKONADI_SEARCH_DEBUG_EXPORT AkonadiSearchDebugSearchPathComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit AkonadiSearchDebugSearchPathComboBox(QWidget *parent = nullptr);
    enum SearchType {
        Contacts = 0,
        ContactCompleter,
        Emails,
        Notes,
        Calendars
    };
    ~AkonadiSearchDebugSearchPathComboBox();

    QString searchPath();

    QString pathFromEnum(SearchType type);
    void setSearchType(SearchType type);
private:
    QString defaultLocations(const QString &dbname);
    void initialize();
};
}
}
#endif // AKONADISEARCHDEBUGSEARCHPATHCOMBOBOX_H
