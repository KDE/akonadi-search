/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2021 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#ifndef AKONADI_SEARCH_PIM_NOTEQUERY_H
#define AKONADI_SEARCH_PIM_NOTEQUERY_H

#include "query.h"
#include "search_pim_export.h"
#include <QString>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
/**
 * Query for a list of contacts matching a criteria
 */
class AKONADI_SEARCH_PIM_EXPORT NoteQuery : public Query
{
public:
    NoteQuery();
    ~NoteQuery() override;

    void matchTitle(const QString &title);
    void matchNote(const QString &note);

    void setLimit(int limit);
    int limit() const;

    ResultIterator exec() override;

private:
    class Private;
    Private *const d;
};
}
}
}

#endif // AKONADI_SEARCH_PIM_NOTEQUERY_H
