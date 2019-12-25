/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef AKONADI_SEARCH_PIM_SEARCHSTORE_H
#define AKONADI_SEARCH_PIM_SEARCHSTORE_H

#include "xapiansearchstore.h"

#include <QSet>

namespace Akonadi {
namespace Search {
class PIMSearchStore : public XapianSearchStore
{
    Q_OBJECT
public:
    explicit PIMSearchStore(QObject *parent = nullptr);

    QStringList types() override;

protected:
    QString findDatabase(const QString &databasePath) const;

    Xapian::Query convertTypes(const QStringList &) override
    {
        return Xapian::Query();
    }

    QByteArray idPrefix() override
    {
        return QByteArray("akonadi");
    }

    virtual Xapian::Query constructQuery(const QString &property, const QVariant &value, Term::Comparator com) override;
    QUrl constructUrl(const Xapian::docid &docid) override;

    QHash<QString, QString> m_prefix;

    /* Simple boolean value
     * value == true -> search for B<name>
     * value == false -> search for BN<name>
     */
    QSet<QString> m_boolProperties;

    /*Search for a boolean value with appended value in name
     * (<name>+<value>) without prefixed B
     */
    QSet<QString> m_boolWithValue;

    QHash<QString, int> m_valueProperties;
};
}
}

#endif // AKONADI_SEARCH_PIM_SEARCHSTORE_H
