/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "search_pim_export.h"
#include <QString>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
// FIXME: Make this async!!
/** Contact completer. */
class AKONADI_SEARCH_PIM_EXPORT ContactCompleter
{
public:
    explicit ContactCompleter(const QString &prefix, int limit = 10);

    Q_REQUIRED_RESULT QStringList complete();

private:
    const QString m_prefix;
    const int m_limit;
};
}
}
}
