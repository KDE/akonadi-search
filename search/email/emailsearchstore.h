/*
 * This file is part of the KDE Akonadi Search Project
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

#ifndef AKONADI_SEARCH_PIM_EMAIL_SEARCHSTORE_H
#define AKONADI_SEARCH_PIM_EMAIL_SEARCHSTORE_H

#include "../pimsearchstore.h"

namespace Akonadi
{
namespace Search
{

class EmailSearchStore : public PIMSearchStore
{
    Q_OBJECT
    Q_INTERFACES(Akonadi::Search::SearchStore)
#ifndef AKONADI_SEARCH_NO_PLUGINS
    Q_PLUGIN_METADATA(IID "org.kde.Akonadi.Search.SearchStore" FILE "emailsearchstore.json")
#endif
public:
    explicit EmailSearchStore(QObject *parent = nullptr);

    QStringList types() Q_DECL_OVERRIDE;
    QString text(int queryId) Q_DECL_OVERRIDE;
    QString icon(int) Q_DECL_OVERRIDE
    {
        return QStringLiteral("internet-mail");
    }

protected:
    virtual Xapian::Query constructQuery(const QString &property, const QVariant &value,
                                         Term::Comparator com) Q_DECL_OVERRIDE;
    Xapian::Query finalizeQuery(const Xapian::Query &query) Q_DECL_OVERRIDE;
};

}
}

#endif // AKONADI_SEARCH_PIM_EMAIL_SEARCHSTORE_H
