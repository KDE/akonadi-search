/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#ifndef AKONADI_SEARCH_CORE_SEARCHSTORE_H
#define AKONADI_SEARCH_CORE_SEARCHSTORE_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QUrl>

#include "search_core_export.h"

namespace Akonadi {
/** Akonadi search infrastructure. */
namespace Search {
class Query;

/** Search store. */
class AKONADI_SEARCH_CORE_EXPORT SearchStore : public QObject
{
    Q_OBJECT
public:
    explicit SearchStore(QObject *parent = nullptr);
    ~SearchStore() override;

    /**
     * Override search stores for testing
     */
    static void overrideSearchStores(const QList<SearchStore *> &overrideSearchStores);

    typedef QList< QSharedPointer<SearchStore> > List;

    /**
     * Gives a list of available search stores. These stores must be managed and
     * deleted by the caller
     */
    static List searchStores();

    /**
     * Returns a list of types which can be searched for
     * in this store
     */
    virtual QStringList types() = 0;

    /**
     * Executes the particular query synchronously.
     *
     * \return Returns a integer representating the integer
     */
    virtual int exec(const Query &query) = 0;
    virtual bool next(int queryId) = 0;
    virtual void close(int queryId) = 0;

    virtual QByteArray id(int queryId) = 0;

    virtual QUrl url(int queryId);
    virtual QString text(int queryId);
    virtual QString icon(int queryId);
    virtual QString property(int queryId, const QString &propName);
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

#endif // AKONADI_SEARCH_CORE_SEARCHSTORE_H
