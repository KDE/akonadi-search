/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2017  Daniel Vrátil <dvratil@kde.org>
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

#ifndef AKONADI_SEARCH_INDEXING_PLUGIN_H_
#define AKONADI_SEARCH_INDEXING_PLUGIN_H_

#include <QObject>
#include <akonadi/abstractindexingplugin.h>

#include <QHash>
#include <QVector>

#include <functional>

class QTimer;

namespace Akonadi {
namespace Search {

class Store;

class IndexingPlugin : public QObject,
                       public Akonadi::Server::AbstractIndexingPlugin
{
    Q_OBJECT
    Q_INTERFACES(Akonadi::Server::AbstractIndexingPlugin)
    Q_PLUGIN_METADATA(IID "org.kde.akonadi.IndexingPlugin" FILE "akonadi_indexing_plugin.json")

public:
    explicit IndexingPlugin();
    ~IndexingPlugin() override;

    bool index(const QString &mimeType, qint64 id, const QByteArray &rawData) override;
    bool copy(const QString &mimeType, qint64 sourceId, qint64 sourceCollection,
              qint64 destId, qint64 destCollection) override;
    bool move(const QString &mimeType, qint64 id, qint64 sourceCollection, qint64 destCollection) override;
    bool removeItem(const QString &mimeType, qint64 id) override;
    bool removeCollection(const QString &mimeType, qint64 id) override;

private:
    using IndexingFunc = std::function<bool(Store*)>;
    bool doIndex(const QString &mimeType, const IndexingFunc &indexFunc);

    QHash<QString, Store*> mStoreCache;
};

}
}

#endif
