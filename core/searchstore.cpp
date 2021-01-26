/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "searchstore.h"
#include "akonadi_search_core_debug.h"

#include <QCoreApplication>
#include <QDir>
#include <QMutex>
#include <QPluginLoader>
#include <QSharedPointer>
#include <QThreadStorage>

using namespace Akonadi::Search;

SearchStore::SearchStore(QObject *parent)
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

QString SearchStore::property(int, const QString &)
{
    return QString();
}

Q_GLOBAL_STATIC(SearchStore::List, s_overrideSearchStores)

void SearchStore::overrideSearchStores(const QList<SearchStore *> &overrideSearchStores)
{
    List *list = &(*s_overrideSearchStores);
    list->clear();
    list->reserve(overrideSearchStores.count());

    for (SearchStore *store : overrideSearchStores) {
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
        qCDebug(AKONADI_SEARCH_CORE_LOG) << "Overriding search stores.";
        return *s_overrideSearchStores;
    }

    // Get all the plugins
    QStringList plugins;
    QStringList pluginPaths;

    const QStringList paths = QCoreApplication::libraryPaths();
    for (const QString &libraryPath : paths) {
        QString path(libraryPath + QStringLiteral("/akonadi"));
        QDir dir(path);

        if (!dir.exists()) {
            continue;
        }

        const QStringList entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QString &fileName : entryList) {
            if (plugins.contains(fileName)) {
                continue;
            }

            plugins << fileName;
            pluginPaths << dir.absoluteFilePath(fileName);
        }
    }
    plugins.clear();

    SearchStore::List stores;
    for (const QString &pluginPath : qAsConst(pluginPaths)) {
        QPluginLoader loader(pluginPath);

        const QVariantMap metadata = loader.metaData().toVariantMap()[QStringLiteral("MetaData")].toMap();
        if (metadata[QStringLiteral("X-Akonadi-PluginType")].toString() != QLatin1String("SearchStore")) {
            continue;
        }

        if (!loader.load()) {
            qCWarning(AKONADI_SEARCH_CORE_LOG) << "Could not create Akonadi Search Store: " << pluginPath;
            qCWarning(AKONADI_SEARCH_CORE_LOG) << loader.errorString();
            continue;
        }

        QObject *obj = loader.instance();
        if (obj) {
            SearchStore *ex = qobject_cast<SearchStore *>(obj);
            if (ex) {
                stores << QSharedPointer<SearchStore>(ex);
            } else {
                qCDebug(AKONADI_SEARCH_CORE_LOG) << "Plugin could not be converted to an Akonadi::Search::SearchStore " << pluginPath;
            }
        } else {
            qCDebug(AKONADI_SEARCH_CORE_LOG) << "Plugin could not create instance" << pluginPath;
        }
    }

    return stores;
}
