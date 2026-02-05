/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "resultiterator.h"
#include "search_core_export.h"

#include <memory>

class QVariant;

namespace Akonadi
{
namespace Search
{
class Term;
class QueryPrivate;

/*!
 * \class Akonadi::Search::Query
 * \inheader AkonadiSearch/Core/Query
 * \inmodule AkonadiSearch
 * \brief Search query class for executing searches in Akonadi.
 *
 * The Query class is used to construct and execute search queries against
 * the Akonadi search index. It allows you to specify search terms, filter
 * by type, set date ranges, and configure sorting options.
 *
 * \sa Term, ResultIterator
 */
class AKONADI_SEARCH_CORE_EXPORT Query
{
public:
    /*!
     * \brief Constructs an empty query.
     */
    Query();
    /*!
     * \brief Constructs a query with the specified search term.
     * \param t The search term to initialize the query with.
     */
    Query(const Term &t);
    /*!
     * \brief Constructs a copy of the query \a rhs.
     * \param rhs The query to copy.
     */
    Query(const Query &rhs);
    /*!
     * \brief Destructs the query.
     */
    ~Query();

    /*!
     * \brief Sets the search term for this query.
     * \param t The search term to set.
     * \sa term()
     */
    void setTerm(const Term &t);
    /*!
     * \brief Returns the search term for this query.
     * \return The current search term.
     * \sa setTerm()
     */
    [[nodiscard]] Term term() const;

    /*!
     * \brief Adds a type filter to the query results.
     * \param type The type to add. Multiple types can be separated with '/'.
     *
     * Each Item in the result must contain one of the types.
     * This is generally used to filter only Files, Emails, Tags, etc.
     *
     * One can add multiple types in one go by separating individual types
     * with a '/'. Eg - "File/Audio".
     *
     * Please note that the types are ANDed together. So searching for "Image"
     * and "Video" will probably never return any results. Have a look at
     * KFileMetaData::TypeInfo for a list of type names.
     * \sa setType(), types()
     */
    void addType(const QString &type);
    /*!
     * \brief Adds multiple type filters to the query results.
     * \param typeList The list of types to add.
     * \sa addType(), setTypes()
     */
    void addTypes(const QStringList &typeList);
    /*!
     * \brief Sets the type filter, replacing any existing type filters.
     * \param type The type to set.
     * \sa addType(), types()
     */
    void setType(const QString &type);
    /*!
     * \brief Sets multiple type filters, replacing any existing type filters.
     * \param types The list of types to set.
     * \sa addTypes(), types()
     */
    void setTypes(const QStringList &types);

    /*!
     * \brief Returns the list of type filters for this query.
     * \return The list of type filters.
     * \sa setType(), setTypes()
     */
    [[nodiscard]] QStringList types() const;

    /*!
     * \brief Sets the search string for this query.
     * \param str The search string to set. Can contain a single word or sentence.
     *
     * Each search backend will interpret it in its own way, and try
     * to give the best possible results.
     * \sa searchString()
     */
    void setSearchString(const QString &str);
    /*!
     * \brief Returns the search string for this query.
     * \return The current search string.
     * \sa setSearchString()
     */
    [[nodiscard]] QString searchString() const;

    /*!
     * \brief Sets the maximum number of results to return.
     * \param limit The maximum number of results. By default the limit is 100000.
     * \sa limit()
     */
    void setLimit(uint limit);
    /*!
     * \brief Returns the maximum number of results for this query.
     * \return The result limit.
     * \sa setLimit()
     */
    [[nodiscard]] uint limit() const;

    /*!
     * \brief Sets the offset for the query results.
     * \param offset The number of results to skip from the beginning.
     * \sa offset()
     */
    void setOffset(uint offset);
    /*!
     * \brief Returns the result offset for this query.
     * \return The result offset.
     * \sa setOffset()
     */
    [[nodiscard]] uint offset() const;

    /*!
     * \brief Filters the results in the specified date range.
     * \param year The year to filter by, or -1 to ignore.
     * \param month The month to filter by, or -1 to ignore. Default is -1.
     * \param day The day to filter by, or -1 to ignore. Default is -1.
     * \sa yearFilter(), monthFilter(), dayFilter()
     */
    void setDateFilter(int year, int month = -1, int day = -1);

    /*!
     * \brief Returns the year filter for this query.
     * \return The year filter, or -1 if not set.
     * \sa setDateFilter()
     */
    [[nodiscard]] int yearFilter() const;
    /*!
     * \brief Returns the month filter for this query.
     * \return The month filter, or -1 if not set.
     * \sa setDateFilter()
     */
    [[nodiscard]] int monthFilter() const;
    /*!
     * \brief Returns the day filter for this query.
     * \return The day filter, or -1 if not set.
     * \sa setDateFilter()
     */
    [[nodiscard]] int dayFilter() const;

    /*!
     * \enum Query::SortingOption
     * \brief Defines the sorting options for query results.
     */
    enum SortingOption : uint8_t {
        /*!
         * The results are returned in the most efficient order. They can
         * be returned in any order.
         */
        SortNone,

        /*!
         * The results are returned in the order the SearchStore decides
         * should be ideal. This criteria could be based on any factors.
         * Read the documentation for the corresponding search store.
         */
        SortAuto,

        /*!
         * The results are returned based on the explicit property specified.
         * The implementation of this depends on the search store.
         */
        SortProperty
    };

    /*!
     * \brief Sets the sorting option for query results.
     * \param option The sorting option to use.
     * \sa sortingOption()
     */
    void setSortingOption(SortingOption option);
    /*!
     * \brief Returns the sorting option for this query.
     * \return The current sorting option.
     * \sa setSortingOption()
     */
    [[nodiscard]] SortingOption sortingOption() const;

    /*!
     * \brief Sets the property to use for sorting query results.
     * \param property The property name to sort by.
     *
     * This automatically sets the sorting mechanism to SortProperty.
     * \sa sortingProperty(), setSortingOption()
     */
    void setSortingProperty(const QString &property);
    /*!
     * \brief Returns the sorting property for this query.
     * \return The current sorting property.
     * \sa setSortingProperty()
     */
    [[nodiscard]] QString sortingProperty() const;

    /*!
     * \brief Adds a custom option to the query.
     * \param option The option name.
     * \param value The option value.
     *
     * Adds a custom option which any search backend could use
     * to configure the query result. Each backend has their own custom
     * options which should be looked up in their corresponding documentation.
     * \sa removeCustomOption(), customOption()
     */
    void addCustomOption(const QString &option, const QVariant &value);
    /*!
     * \brief Removes a custom option from the query.
     * \param option The option name to remove.
     * \sa addCustomOption()
     */
    void removeCustomOption(const QString &option);
    /*!
     * \brief Returns the value of a custom option.
     * \param option The option name.
     * \return The option value, or an empty QVariant if not found.
     * \sa addCustomOption()
     */
    [[nodiscard]] QVariant customOption(const QString &option) const;
    /*!
     * \brief Returns all custom options for this query.
     * \return A map of all custom options and their values.
     * \sa addCustomOption()
     */
    [[nodiscard]] QVariantMap customOptions() const;

    /*!
     * \brief Executes the query and returns the results.
     * \return An iterator over the query results.
     * \sa ResultIterator
     */
    [[nodiscard]] ResultIterator exec();

    /*!
     * \brief Converts the query to JSON representation.
     * \return A JSON representation of the query.
     * \sa fromJSON()
     */
    [[nodiscard]] QByteArray toJSON() const;
    /*!
     * \brief Creates a query from a JSON representation.
     * \param arr The JSON representation of a query.
     * \return A query reconstructed from the JSON data.
     * \sa toJSON()
     */
    static Query fromJSON(const QByteArray &arr);

    /*!
     * \brief Converts the query to a search URL.
     * \param title The title to include in the URL. Optional.
     * \return A URL representation of the query.
     * \sa fromSearchUrl()
     */
    [[nodiscard]] QUrl toSearchUrl(const QString &title = QString());
    /*!
     * \brief Creates a query from a search URL.
     * \param url The search URL.
     * \return A query reconstructed from the URL.
     * \sa toSearchUrl()
     */
    static Query fromSearchUrl(const QUrl &url);
    /*!
     * \brief Extracts the title from a query URL.
     * \param url The search URL.
     * \return The title embedded in the URL.
     * \sa toSearchUrl()
     */
    static QString titleFromQueryUrl(const QUrl &url);

    /*!
     * \brief Compares this query with another query for equality.
     * \param rhs The query to compare with.
     * \return \c true if the queries are equal, \c false otherwise.
     */
    bool operator==(const Query &rhs) const;

    /*!
     * \brief Assigns the contents of \a rhs to this query.
     * \param rhs The query to copy from.
     * \return A reference to this query.
     */
    Query &operator=(const Query &rhs);

private:
    std::unique_ptr<QueryPrivate> const d;
};
}
}
