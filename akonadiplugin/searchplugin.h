/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include <QObject>
#include <QStringList>
#include <akonadi/abstractsearchplugin.h>

namespace Akonadi
{
namespace Search
{
class Query;
}
}

class SearchPlugin : public QObject, public Akonadi::AbstractSearchPlugin
{
    Q_OBJECT
    Q_INTERFACES(Akonadi::AbstractSearchPlugin)
    Q_PLUGIN_METADATA(IID "org.kde.akonadi.SearchPlugin" FILE "akonadi_search_plugin.json")
public:
    [[nodiscard]] QSet<qint64> search(const QString &query, const QList<qint64> &collections, const QStringList &mimeTypes) override;
};
