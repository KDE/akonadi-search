/*
  SPDX-FileCopyrightText: 2014-2025 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "search_debug_export.h"
#include <QComboBox>
namespace Akonadi
{
namespace Search
{
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
        Calendars,
    };
    ~AkonadiSearchDebugSearchPathComboBox() override;

    [[nodiscard]] QString searchPath() const;

    [[nodiscard]] QString pathFromEnum(SearchType type) const;
    void setSearchType(SearchType type);

private:
    AKONADI_SEARCH_DEBUG_NO_EXPORT const QString defaultLocations(const QString &dbname) const;
    AKONADI_SEARCH_DEBUG_NO_EXPORT void initialize();
};
}
}
