/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

#include "search_core_export.h"

namespace Akonadi
{
/*! Akonadi search infrastructure. */
namespace Search
{
class Query;

/*!
 * \class Akonadi::Search::SearchStore
 * \inheader AkonadiSearch/Core/SearchStore
 * \inmodule AkonadiSearch
 * \brief Base class for search store implementations.
 *
 * SearchStore is an abstract base class that defines the interface for search
 * store implementations. Search stores are responsible for executing queries
 * and returning search results. Different implementations may use different
 * backends (e.g., Xapian, SQLite, etc.).
 *
 * \sa Query, ResultIterator
 */
class AKONADI_SEARCH_CORE_EXPORT SearchStore : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a search store with the given parent.
     * \param parent The parent object. Ownership is taken by parent.
     */
    explicit SearchStore(QObject *parent = nullptr);
    /*!
     * \brief Destructs the search store.
     */
    ~SearchStore() override;

    /*!
     * \brief Overrides the search stores for testing purposes.
     * \param overrideSearchStores The list of search store instances to use for testing.
     */
    static void overrideSearchStores(const QList<SearchStore *> &overrideSearchStores);

    /*!
     * \typedef SearchStore::List
     * \brief A list of shared pointers to SearchStore instances.
     */
    using List = QList<QSharedPointer<SearchStore>>;

    /*!
     * \brief Returns a list of available search stores.
     *
     * These stores must be managed and deleted by the caller.
     * \return A list of available search store instances.
     * \sa overrideSearchStores()
     */
    static List searchStores();

    /*!
     * \brief Returns a list of types which can be searched for in this store.
     * \return A list of searchable type names (e.g., "File", "Email", etc.).
     */
    virtual QStringList types() = 0;

    /*!
     * \brief Executes the specified query synchronously.
     * \param query The query to execute.
     * \return An integer representing the query ID, or -1 on error.
     * \sa next(), close()
     */
    virtual int exec(const Query &query) = 0;
    /*!
     * \brief Advances to the next result for the given query.
     * \param queryId The query ID returned by exec().
     * \return \c true if there is a next result, \c false if no more results.
     * \sa exec(), close()
     */
    virtual bool next(int queryId) = 0;
    /*!
     * \brief Closes the specified query and frees its resources.
     * \param queryId The query ID returned by exec().
     * \sa exec()
     */
    virtual void close(int queryId) = 0;

    /*!
     * \brief Returns the ID of the current result.
     * \param queryId The query ID returned by exec().
     * \return The ID of the current result.
     */
    [[nodiscard]] virtual QByteArray id(int queryId) = 0;

    /*!
     * \brief Returns the URL of the current result.
     * \param queryId The query ID returned by exec().
     * \return The URL of the current result.
     * \sa text(), icon()
     */
    [[nodiscard]] virtual QUrl url(int queryId);
    /*!
     * \brief Returns the text representation of the current result.
     * \param queryId The query ID returned by exec().
     * \return The text representation of the current result.
     * \sa url(), icon()
     */
    [[nodiscard]] virtual QString text(int queryId);
    /*!
     * \brief Returns the icon name for the current result.
     * \param queryId The query ID returned by exec().
     * \return The icon name for the current result.
     * \sa url(), text()
     */
    [[nodiscard]] virtual QString icon(int queryId);
    /*!
     * \brief Returns a custom property of the current result.
     * \param queryId The query ID returned by exec().
     * \param propName The name of the property to retrieve.
     * \return The value of the specified property.
     */
    [[nodiscard]] virtual QString property(int queryId, const QString &propName);
};

//
// Convenience functions
//
inline QByteArray serialize(const QByteArray &namespace_, int id)
{
    return namespace_ + ':' + QByteArray::number(id);
}

inline int deserialize(const QByteArray &namespace_, const QByteArray &str)
{
    // The +1 is for the ':'
    return str.mid(namespace_.size() + 1).toInt();
}
} // namespace Search
} // namespace Akonadi

Q_DECLARE_INTERFACE(Akonadi::Search::SearchStore, "org.kde.Akonadi.Search.SearchStore")
