/*
 * SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include <Akonadi/Item>
#include <QStringList>

namespace Akonadi
{
class Collection;
}

class AbstractIndexer
{
public:
    AbstractIndexer();
    virtual ~AbstractIndexer();

    virtual QStringList mimeTypes() const = 0;
    virtual void index(const Akonadi::Item &item) = 0;
    virtual void remove(const Akonadi::Item &item) = 0;
    virtual void remove(const Akonadi::Collection &item) = 0;
    virtual void commit() = 0;

    virtual void move(Akonadi::Item::Id item, Akonadi::Collection::Id from, Akonadi::Collection::Id to);
    virtual void updateFlags(const Akonadi::Item &item, const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removed);
};

