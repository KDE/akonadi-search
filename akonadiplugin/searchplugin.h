/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#ifndef AKONADI_SEARCH_PIM_SEARCHPLUGIN_H
#define AKONADI_SEARCH_PIM_SEARCHPLUGIN_H

#include <QStringList>
#include <akonadi/abstractsearchplugin.h>
#include <QObject>

namespace Akonadi {
namespace Search {
class Query;
}
}

class SearchPlugin : public QObject, public Akonadi::AbstractSearchPlugin
{
    Q_OBJECT
    Q_INTERFACES(Akonadi::AbstractSearchPlugin)
    Q_PLUGIN_METADATA(IID "org.kde.akonadi.SearchPlugin" FILE "akonadi_search_plugin.json")
public:
    QSet<qint64> search(const QString &query, const QVector<qint64> &collections, const QStringList &mimeTypes) override;
};

#endif
