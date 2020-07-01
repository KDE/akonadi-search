/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#ifndef AKONADI_SEARCH_PIM_CONTACTCOMPLETER_H
#define AKONADI_SEARCH_PIM_CONTACTCOMPLETER_H

#include <QString>
#include "search_pim_export.h"

namespace Akonadi {
namespace Search {
namespace PIM {
// FIXME: Make this async!!
/** Contact completer. */
class AKONADI_SEARCH_PIM_EXPORT ContactCompleter
{
public:
    explicit ContactCompleter(const QString &prefix, int limit = 10);

    QStringList complete();

private:
    QString m_prefix;
    int m_limit;
};
}
}
}
#endif // AKONADI_SEARCH_PIM_CONTACTCOMPLETER_H
