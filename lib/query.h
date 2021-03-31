/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "search_pim_export.h"
#include <QByteArray>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
class ResultIterator;

/** Query base class. */
class AKONADI_SEARCH_PIM_EXPORT Query
{
public:
    Query();
    virtual ~Query();
    virtual ResultIterator exec() = 0;

    static Query *fromJSON(const QByteArray &json);
    static QString defaultLocation(const QString &dbName);
};
}
}
}

