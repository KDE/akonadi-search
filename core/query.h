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

class QVariant;

namespace Akonadi
{
namespace Search
{
class Term;

/** Search query. */
class AKONADI_SEARCH_CORE_EXPORT Query
{
public:
    Query();
    Query(const Term &t);
    Query(const Query &rhs);
    ~Query();

    void setTerm(const Term &t);
    Term term() const;

    /**
     * Add a type to the results of the query.
     *
     * Each Item in the result must contain one of the types.
     * This is generally used to filter only Files, Emails, Tags, etc
     *
     * One can add multiple types in one go by separating individual types
     * with a '/'. Eg - "File/Audio".
     *
     * Please note that the types are ANDed together. So searching for "Image"
     * and "Video" will probably never return any results. Have a look at
     * KFileMetaData::TypeInfo for a list of type names.
     */
    void addType(const QString &type);
    void addTypes(const QStringList &typeList);
    void setType(const QString &type);
    void setTypes(const QStringList &types);

    QStringList types() const;

    /**
     * Set some text which should be used to search for Items. This
     * contain a single word or an entire sentence.
     *
     * Each search backend will interpret it in its own way, and try
     * to give the best possible results.
     */
    void setSearchString(const QString &str);
    QString searchString() const;

    /**
     * Only a maximum of \p limit results will be returned.
     * By default the limit is 100000.
     */
    void setLimit(uint limit);
    uint limit() const;

    void setOffset(uint offset);
    uint offset() const;

    /**
     * Filter the results in the specified date range.
     *
     * The year/month/day may be set to -1 in order to ignore it.
     */
    void setDateFilter(int year, int month = -1, int day = -1);

    int yearFilter() const;
    int monthFilter() const;
    int dayFilter() const;

    enum SortingOption {
        /**
         * The results are returned in the most efficient order. They can
         * be returned in any order.
         */
        SortNone,

        /**
         * The results are returned in the order the SearchStore decides
         * should be ideal. This criteria could be based on any factors.
         * Read the documentation for the corresponding search store.
         */
        SortAuto,

        /**
         * The results are returned based on the explicit property specified.
         * The implementation of this depends on the search store.
         */
        SortProperty
    };

    void setSortingOption(SortingOption option);
    SortingOption sortingOption() const;

    /**
     * Sets the property that should be used for sorting. This automatically
     * set the sorting mechanism to SortProperty
     */
    void setSortingProperty(const QString &property);
    QString sortingProperty() const;

    /**
     * Adds a custom option which any search backend could use
     * to configure the query result.
     *
     * Each backend has their own custom options which should be
     * looked up in their corresponding documentation
     */
    void addCustomOption(const QString &option, const QVariant &value);
    void removeCustomOption(const QString &option);
    QVariant customOption(const QString &option) const;
    QVariantMap customOptions() const;

    ResultIterator exec();

    QByteArray toJSON() const;
    static Query fromJSON(const QByteArray &arr);

    QUrl toSearchUrl(const QString &title = QString());
    static Query fromSearchUrl(const QUrl &url);
    static QString titleFromQueryUrl(const QUrl &url);

    bool operator==(const Query &rhs) const;

    Query &operator=(const Query &rhs);

private:
    class Private;
    Private *const d;
};
}
}

