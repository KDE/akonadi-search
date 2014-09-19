/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "searchstore.h"

#include <QDebug>
#include <QThreadStorage>
#include <QMutex>
#include <QSharedPointer>
#include <QCoreApplication>
#include <QPluginLoader>
#include <QDir>

using namespace Baloo;

SearchStore::SearchStore(QObject* parent)
    : QObject(parent)
{
}

SearchStore::~SearchStore()
{
}

QUrl SearchStore::url(int)
{
    return QUrl();
}

QString SearchStore::icon(int)
{
    return QString();
}

QString SearchStore::text(int)
{
    return QString();
}

QString SearchStore::property(int, const QString&)
{
    return QString();
}

Q_GLOBAL_STATIC(SearchStore::List, s_overrideSearchStores)

void SearchStore::overrideSearchStores(const QList<SearchStore*> &overrideSearchStores)
{
    List* list = &(*s_overrideSearchStores);
    list->clear();

    Q_FOREACH (SearchStore* store, overrideSearchStores) {
        list->append(QSharedPointer<SearchStore>(store));
    }
}

//
// Search Stores
//
// static
SearchStore::List SearchStore::searchStores()
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);

    if (s_overrideSearchStores && !s_overrideSearchStores->isEmpty()) {
        qDebug() << "Overriding search stores.";
        return *s_overrideSearchStores;
    }

    // Get all the plugins
    QStringList plugins;
    QStringList pluginPaths;

    QStringList paths = QCoreApplication::libraryPaths();
    Q_FOREACH (const QString& libraryPath, paths) {
        QString path(libraryPath + QStringLiteral("/kf5/baloo"));
        QDir dir(path);

        if (!dir.exists()) {
            continue;
        }

        QStringList entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        Q_FOREACH (const QString& fileName, entryList) {
            if (plugins.contains(fileName))
                continue;

            plugins << fileName;
            pluginPaths << dir.absoluteFilePath(fileName);
        }
    }
    plugins.clear();

    SearchStore::List stores;
    Q_FOREACH (const QString& pluginPath, pluginPaths) {
        QPluginLoader loader(pluginPath);

        if (!loader.load()) {
            qWarning() << "Could not create Baloo Search Store: " << pluginPath;
            qWarning() << loader.errorString();
            continue;
        }

        QObject* obj = loader.instance();
        if (obj) {
            SearchStore* ex = qobject_cast<SearchStore*>(obj);
            if (ex) {
                stores << QSharedPointer<SearchStore>(ex);
            } else {
                qDebug() << "Plugin could not be converted to an Baloo::SearchStore";
                qDebug() << pluginPath;
            }
        }
        else {
            qDebug() << "Plugin could not create instance" << pluginPath;
        }
    }

    return stores;
}
