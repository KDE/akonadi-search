/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

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

    QStringList types() override;
    QString text(int queryId) override;
    QString icon(int) override
    {
        return QStringLiteral("internet-mail");
    }

protected:
    Xapian::Query constructQuery(const QString &property, const QVariant &value, Term::Comparator com) override;
    Xapian::Query finalizeQuery(const Xapian::Query &query) override;
};
}
}

