/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "xapiansearchstore.h"

#include <QSet>

namespace Akonadi
{
namespace Search
{
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
        return {};
    }

    QByteArray idPrefix() override
    {
        return {"akonadi"};
    }

    Xapian::Query constructQuery(const QString &property, const QVariant &value, Term::Comparator com) override;
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
