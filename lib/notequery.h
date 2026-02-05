/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2026 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "query.h"
#include "search_pim_export.h"
#include <QString>

#include <memory>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
class NoteQueryPrivate;

/*!
 * \class Akonadi::Search::PIM::NoteQuery
 * \inheader AkonadiSearch/PIM/NoteQuery
 * \inmodule AkonadiSearchPIM
 * \brief Search query for notes.
 *
 * NoteQuery allows searching for notes by title and content.
 *
 * \sa Query, ResultIterator
 */
class AKONADI_SEARCH_PIM_EXPORT NoteQuery : public Query
{
public:
    /*!
     * \brief Constructs an empty note query.
     */
    NoteQuery();
    /*!
     * \brief Destructs the note query.
     */
    ~NoteQuery() override;

    /*!
     * \brief Filters notes by title.
     * \param title The title to match.
     */
    void matchTitle(const QString &title);
    /*!
     * \brief Filters notes by content.
     * \param note The content to match.
     */
    void matchNote(const QString &note);

    /*!
     * \brief Sets the maximum number of results to return.
     * \param limit The result limit.
     */
    void setLimit(int limit);
    /*!
     * \brief Returns the maximum number of results for this query.
     * \return The result limit.
     */
    [[nodiscard]] int limit() const;

    /*!
     * \brief Executes the query and returns an iterator to fetch results.
     * \return An iterator over the search results.
     */
    [[nodiscard]] ResultIterator exec() override;

private:
    std::unique_ptr<NoteQueryPrivate> const d;
};
}
}
}
