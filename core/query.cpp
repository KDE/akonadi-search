/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "query.h"

#include "searchstore.h"
#include "term.h"

#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QUrlQuery>
#include <QVariant>

#include <QJsonDocument>
#include <QJsonObject>

using namespace Qt::Literals::StringLiterals;
using namespace Akonadi::Search;

const int defaultLimit = 100000;

class Akonadi::Search::QueryPrivate
{
public:
    Term m_term;

    QStringList m_types;
    QString m_searchString;
    uint m_limit = defaultLimit;
    uint m_offset = 0;

    int m_yearFilter = -1;
    int m_monthFilter = -1;
    int m_dayFilter = -1;

    Query::SortingOption m_sortingOption = Query::SortAuto;
    QString m_sortingProperty;
    QVariantMap m_customOptions;
};

Query::Query()
    : d(new QueryPrivate)
{
}

Query::Query(const Term &t)
    : d(new QueryPrivate)
{
    d->m_term = t;
}

Query::Query(const Query &rhs)
    : d(new QueryPrivate(*rhs.d))
{
}

Query::~Query() = default;

void Query::setTerm(const Term &t)
{
    d->m_term = t;
}

Term Query::term() const
{
    return d->m_term;
}

void Query::addType(const QString &type)
{
    d->m_types << type.split(u'/', Qt::SkipEmptyParts);
}

void Query::addTypes(const QStringList &typeList)
{
    for (const QString &type : typeList) {
        addType(type);
    }
}

void Query::setType(const QString &type)
{
    d->m_types.clear();
    addType(type);
}

void Query::setTypes(const QStringList &types)
{
    d->m_types = types;
}

QStringList Query::types() const
{
    return d->m_types;
}

QString Query::searchString() const
{
    return d->m_searchString;
}

void Query::setSearchString(const QString &str)
{
    d->m_searchString = str;
}

uint Query::limit() const
{
    return d->m_limit;
}

void Query::setLimit(uint limit)
{
    d->m_limit = limit;
}

uint Query::offset() const
{
    return d->m_offset;
}

void Query::setOffset(uint offset)
{
    d->m_offset = offset;
}

void Query::setDateFilter(int year, int month, int day)
{
    d->m_yearFilter = year;
    d->m_monthFilter = month;
    d->m_dayFilter = day;
}

int Query::yearFilter() const
{
    return d->m_yearFilter;
}

int Query::monthFilter() const
{
    return d->m_monthFilter;
}

int Query::dayFilter() const
{
    return d->m_dayFilter;
}

void Query::setSortingOption(Query::SortingOption option)
{
    d->m_sortingOption = option;
}

Query::SortingOption Query::sortingOption() const
{
    return d->m_sortingOption;
}

void Query::setSortingProperty(const QString &property)
{
    d->m_sortingOption = SortProperty;
    d->m_sortingProperty = property;
}

QString Query::sortingProperty() const
{
    return d->m_sortingProperty;
}

void Query::addCustomOption(const QString &option, const QVariant &value)
{
    d->m_customOptions.insert(option, value);
}

QVariant Query::customOption(const QString &option) const
{
    return d->m_customOptions.value(option);
}

QVariantMap Query::customOptions() const
{
    return d->m_customOptions;
}

void Query::removeCustomOption(const QString &option)
{
    d->m_customOptions.remove(option);
}

Q_GLOBAL_STATIC_WITH_ARGS(SearchStore::List, s_searchStores, (SearchStore::searchStores()))

ResultIterator Query::exec()
{
    // vHanda: Maybe this should default to allow searches on all search stores?
    Q_ASSERT_X(!types().isEmpty(), "Akonadi::Search::Query::exec", "A query is being initialized without a type");
    if (types().isEmpty()) {
        return {};
    }

    SearchStore *storeMatch = nullptr;
    for (const QSharedPointer<SearchStore> &store : std::as_const(*s_searchStores)) {
        bool matches = true;
        const auto typeList{types()};
        for (const QString &type : typeList) {
            if (!store->types().contains(type)) {
                matches = false;
                break;
            }
        }

        if (matches) {
            storeMatch = store.data();
            break;
        }
    }

    if (!storeMatch) {
        return {};
    }

    int id = storeMatch->exec(*this);
    return {id, storeMatch};
}

QByteArray Query::toJSON() const
{
    QVariantMap map;

    if (!d->m_types.isEmpty()) {
        map[u"type"_s] = d->m_types;
    }

    if (d->m_limit != defaultLimit) {
        map[u"limit"_s] = d->m_limit;
    }

    if (d->m_offset) {
        map[u"offset"_s] = d->m_offset;
    }

    if (!d->m_searchString.isEmpty()) {
        map[u"searchString"_s] = d->m_searchString;
    }

    if (d->m_term.isValid()) {
        map[u"term"_s] = QVariant(d->m_term.toVariantMap());
    }

    if (d->m_yearFilter >= 0) {
        map[u"yearFilter"_s] = d->m_yearFilter;
    }
    if (d->m_monthFilter >= 0) {
        map[u"monthFilter"_s] = d->m_monthFilter;
    }
    if (d->m_dayFilter >= 0) {
        map[u"dayFilter"_s] = d->m_dayFilter;
    }

    if (d->m_sortingOption != SortAuto) {
        map[u"sortingOption"_s] = static_cast<int>(d->m_sortingOption);
    }
    if (!d->m_sortingProperty.isEmpty()) {
        map[u"sortingProperty"_s] = d->m_sortingProperty;
    }

    if (!d->m_customOptions.isEmpty()) {
        map[u"customOptions"_s] = d->m_customOptions;
    }

    QJsonObject jo = QJsonObject::fromVariantMap(map);
    QJsonDocument jdoc;
    jdoc.setObject(jo);
    return jdoc.toJson();
}

// static
Query Query::fromJSON(const QByteArray &arr)
{
    QJsonDocument jdoc = QJsonDocument::fromJson(arr);
    const QVariantMap map = jdoc.object().toVariantMap();

    Query query;
    query.d->m_types = map[u"type"_s].toStringList();

    if (map.contains(u"limit"_s)) {
        query.d->m_limit = map[u"limit"_s].toUInt();
    } else {
        query.d->m_limit = defaultLimit;
    }

    query.d->m_offset = map[u"offset"_s].toUInt();
    query.d->m_searchString = map[u"searchString"_s].toString();
    query.d->m_term = Term::fromVariantMap(map[u"term"_s].toMap());

    if (map.contains(u"yearFilter"_s)) {
        query.d->m_yearFilter = map[u"yearFilter"_s].toInt();
    }
    if (map.contains(u"monthFilter"_s)) {
        query.d->m_monthFilter = map[u"monthFilter"_s].toInt();
    }
    if (map.contains(u"dayFilter"_s)) {
        query.d->m_dayFilter = map[u"dayFilter"_s].toInt();
    }

    if (map.contains(u"sortingOption"_s)) {
        int option = map.value(u"sortingOption"_s).toInt();
        query.d->m_sortingOption = static_cast<SortingOption>(option);
    }

    if (map.contains(u"sortingProperty"_s)) {
        query.d->m_sortingProperty = map.value(u"sortingProperty"_s).toString();
    }

    if (map.contains(u"customOptions"_s)) {
        QVariant var = map[u"customOptions"_s];
        if (var.userType() == QMetaType::QVariantMap) {
            query.d->m_customOptions = map[u"customOptions"_s].toMap();
        } else if (var.userType() == QMetaType::QVariantHash) {
            QVariantHash hash = var.toHash();

            QHash<QString, QVariant>::const_iterator it = hash.constBegin();
            const QHash<QString, QVariant>::const_iterator end = hash.constEnd();
            for (; it != end; ++it) {
                query.d->m_customOptions.insert(it.key(), it.value());
            }
        }
    }

    return query;
}

QUrl Query::toSearchUrl(const QString &title)
{
    QUrl url;
    url.setScheme(u"akonadisearch"_s);

    QUrlQuery urlQuery;
    urlQuery.addQueryItem(u"json"_s, QString::fromUtf8(toJSON()));

    if (!title.isEmpty()) {
        urlQuery.addQueryItem(u"title"_s, title);
    }

    url.setQuery(urlQuery);
    return url;
}

Query Query::fromSearchUrl(const QUrl &url)
{
    if (url.scheme() != "akonadisearch"_L1) {
        return {};
    }

    QUrlQuery urlQuery(url);
    const QString jsonString = urlQuery.queryItemValue(u"json"_s, QUrl::FullyDecoded);
    return Query::fromJSON(jsonString.toUtf8());
}

QString Query::titleFromQueryUrl(const QUrl &url)
{
    QUrlQuery urlQuery(url);
    return urlQuery.queryItemValue(u"title"_s, QUrl::FullyDecoded);
}

bool Query::operator==(const Query &rhs) const
{
    if (rhs.d->m_limit != d->m_limit || rhs.d->m_offset != d->m_offset || rhs.d->m_dayFilter != d->m_dayFilter || rhs.d->m_monthFilter != d->m_monthFilter
        || rhs.d->m_yearFilter != d->m_yearFilter || rhs.d->m_customOptions != d->m_customOptions || rhs.d->m_searchString != d->m_searchString
        || rhs.d->m_sortingProperty != d->m_sortingProperty || rhs.d->m_sortingOption != d->m_sortingOption) {
        return false;
    }

    if (rhs.d->m_types.size() != d->m_types.size()) {
        return false;
    }

    for (const QString &type : std::as_const(rhs.d->m_types)) {
        if (!d->m_types.contains(type)) {
            return false;
        }
    }

    return d->m_term == rhs.d->m_term;
}

Query &Query::operator=(const Query &rhs)
{
    *d = *rhs.d;
    return *this;
}
