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
using namespace Qt::Literals::StringLiterals;

SearchStore::SearchStore(QObject *parent)
    : QObject(parent)
{
}

SearchStore::~SearchStore() = default;

QUrl SearchStore::url(int)
{
    return {};
}

QString SearchStore::icon(int)
{
    return {};
}

QString SearchStore::text(int)
{
    return {};
}

QString SearchStore::property(int, const QString &)
{
    return {};
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
        const QString path(libraryPath + QStringLiteral("/pim6/akonadi"));
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
    for (const QString &pluginPath : std::as_const(pluginPaths)) {
        QPluginLoader loader(pluginPath);

        const QVariantMap metadata = loader.metaData().toVariantMap()[QStringLiteral("MetaData")].toMap();
        if (metadata[QStringLiteral("X-Akonadi-PluginType")].toString() != "SearchStore"_L1) {
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
                qCDebug(AKONADI_SEARCH_CORE_LOG) << " Loaded plugins: " << pluginPath;
            } else {
                qCDebug(AKONADI_SEARCH_CORE_LOG) << "Plugin could not be converted to an Akonadi::Search::SearchStore " << pluginPath;
            }
        } else {
            qCDebug(AKONADI_SEARCH_CORE_LOG) << "Plugin could not create instance" << pluginPath;
        }
    }
    return stores;
}

#include "moc_searchstore.cpp"
