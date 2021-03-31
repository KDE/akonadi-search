/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include "abstractindexer.h"
#include "collectionindexer.h"
#include <AkonadiCore/Collection>
#include <AkonadiCore/item.h>
#include <QObject>
#include <QTimer>
namespace Akonadi
{
namespace Search
{
namespace PIM
{
class IndexedItems;
}
}
}
/**
 * Maintains the various indexers and databases
 */
class Index : public QObject
{
    Q_OBJECT
public:
    explicit Index(QObject *parent = nullptr);
    ~Index() override;

    virtual void removeDatabase();
    virtual bool createIndexers();

    virtual void index(const Akonadi::Item &item);
    virtual void move(const Akonadi::Item::List &items, const Akonadi::Collection &from, const Akonadi::Collection &to);
    virtual void updateFlags(const Akonadi::Item::List &items, const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removed);
    virtual void remove(const QSet<Akonadi::Item::Id> &ids, const QStringList &mimeTypes);
    virtual void remove(const Akonadi::Item::List &items);

    virtual void index(const Akonadi::Collection &collection);
    virtual void change(const Akonadi::Collection &collection);
    virtual void remove(const Akonadi::Collection &col);
    virtual void move(const Akonadi::Collection &collection, const Akonadi::Collection &from, const Akonadi::Collection &to);

    virtual bool haveIndexerForMimeTypes(const QStringList &);
    virtual qlonglong indexedItems(const qlonglong id);
    virtual void findIndexed(QSet<Akonadi::Item::Id> &indexed, Akonadi::Collection::Id);
    virtual void scheduleCommit();

    /// For testing
    void setOverrideDbPrefixPath(const QString &path);

public Q_SLOTS:
    virtual void commit();

private:
    void addIndexer(AbstractIndexer *indexer);
    AbstractIndexer *indexerForItem(const Akonadi::Item &item) const;
    QList<AbstractIndexer *> indexersForMimetypes(const QStringList &mimeTypes) const;

    QList<AbstractIndexer *> m_listIndexer;
    QHash<QString, AbstractIndexer *> m_indexer;
    Akonadi::Search::PIM::IndexedItems *m_indexedItems = nullptr;
    QTimer m_commitTimer;
    CollectionIndexer *m_collectionIndexer = nullptr;
};

