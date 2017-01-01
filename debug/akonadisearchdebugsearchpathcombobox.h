/*
  Copyright (c) 2014-2017 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef AKONADISEARCHDEBUGSEARCHPATHCOMBOBOX_H
#define AKONADISEARCHDEBUGSEARCHPATHCOMBOBOX_H

#include <QComboBox>
#include "search_debug_export.h"
namespace Akonadi
{
namespace Search
{

class AKONADI_SEARCH_DEBUG_EXPORT AkonadiSearchDebugSearchPathComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit AkonadiSearchDebugSearchPathComboBox(QWidget *parent = Q_NULLPTR);
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

