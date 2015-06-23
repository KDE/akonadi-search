/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2014  Christian Mollekopf <mollekopf@kolabsys.com>
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

#ifndef AKONADI_SEARCH_PIM_SEARCHPLUGIN_H
#define AKONADI_SEARCH_PIM_SEARCHPLUGIN_H

#include <QStringList>
#include <akonadi/abstractsearchplugin.h>
#include <QObject>

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
    QSet<qint64> search(const QString &query, const QList<qint64> &collections, const QStringList &mimeTypes) Q_DECL_OVERRIDE;
private:
    Akonadi::Search::Query fromAkonadiQuery(const QString &akonadiQuery, const QList<qint64> &collections, const QStringList &mimeTypes);
};

#endif
