/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2016-2026 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */
#pragma once

#include "search_pim_export.h"
#include <Akonadi/Item>
#include <QObject>

#include <memory>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
class IndexedItemsPrivate;

/*!
 * \class Akonadi::Search::PIM::IndexedItems
 * \inheader AkonadiSearch/PIM/IndexedItems
 * \inmodule AkonadiSearchPIM
 * \brief Provides information about indexed items in search databases.
 *
 * IndexedItems helps manage and query information about items that have been
 * indexed in the Akonadi search system.
 *
 */
class AKONADI_SEARCH_PIM_EXPORT IndexedItems : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs an indexed items manager with the given parent.
     * \param parent The parent object. Ownership is taken by parent.
     */
    explicit IndexedItems(QObject *parent = nullptr);
    /*!
     * \brief Destructs the indexed items manager.
     */
    ~IndexedItems() override;

    /*!
     * \brief Sets the override path for the database prefix.
     * \param path The database prefix path to use.
     */
    void setOverrideDbPrefixPath(const QString &path);

    /*!
     * \brief Returns the number of indexed items for the given ID.
     * \param id The item or collection ID.
     * \return The number of indexed items.
     */
    [[nodiscard]] qlonglong indexedItems(const qlonglong id);

    /*!
     * \brief Finds indexed items in the database.
     * \param indexed The set to populate with indexed item IDs.
     * \param collectionId The collection ID to search in.
     * \param dbPath The database path.
     */
    void findIndexedInDatabase(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId, const QString &dbPath);
    /*!
     * \brief Finds indexed items in the collection.
     * \param indexed The set to populate with indexed item IDs.
     * \param collectionId The collection ID to search in.
     */
    void findIndexed(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id collectionId);

    /*!
     * \brief Returns the email indexing path.
     * \return The path to the email index database.
     */
    [[nodiscard]] QString emailIndexingPath() const;
    /*!
     * \brief Returns the collection indexing path.
     * \return The path to the collection index database.
     */
    [[nodiscard]] QString collectionIndexingPath() const;
    /*!
     * \brief Returns the calendar indexing path.
     * \return The path to the calendar index database.
     */
    [[nodiscard]] QString calendarIndexingPath() const;
    /*!
     * \brief Returns the Akonotes indexing path.
     * \return The path to the Akonotes index database.
     */
    [[nodiscard]] QString akonotesIndexingPath() const;
    /*!
     * \brief Returns the email contacts indexing path.
     * \return The path to the email contacts index database.
     */
    [[nodiscard]] QString emailContactsIndexingPath() const;
    /*!
     * \brief Returns the contact indexing path.
     * \return The path to the contact index database.
     */
    [[nodiscard]] QString contactIndexingPath() const;

private:
    std::unique_ptr<IndexedItemsPrivate> const d;
};
}
}
}
