/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include <Akonadi/Collection>
#include <QString>

namespace Xapian
{
class WritableDatabase;
}

class CollectionIndexer : public QObject
{
    Q_OBJECT
public:
    explicit CollectionIndexer(const QString &path);
    ~CollectionIndexer() override;

    void index(const Akonadi::Collection &collection);
    void change(const Akonadi::Collection &collection);
    void remove(const Akonadi::Collection &col);
    void move(const Akonadi::Collection &collection, const Akonadi::Collection &from, const Akonadi::Collection &to);
    void commit();

private:
    Xapian::WritableDatabase *m_db;
};
