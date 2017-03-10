/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2014-2017 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AKONADI_SEARCH_PIM_NOTEQUERY_H
#define AKONADI_SEARCH_PIM_NOTEQUERY_H

#include "search_pim_export.h"
#include <QString>
#include "query.h"

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
    ~NoteQuery();

    void matchTitle(const QString &title);
    void matchNote(const QString &note);

    void setLimit(int limit);
    int limit() const;

    ResultIterator exec() Q_DECL_OVERRIDE;

private:
    class Private;
    Private *d;
};

}
}
}

#endif // AKONADI_SEARCH_PIM_NOTEQUERY_H
