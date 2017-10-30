/*
 * Copyright (C) 2017  Daniel Vrátil <dvratil@kde.org>
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

#ifndef AKONADISEARCH_STORE_H_
#define AKONADISEARCH_STORE_H_

#include "akonadisearch_export.h"

#include <QVector>

namespace Akonadi {

namespace Search {

class ResultIterator;
class StorePrivate;
class AKONADISEARCH_EXPORT Store 
{
public:
    enum OpenMode {
        ReadOnly,
        WriteOnly
    };

    static QVector<Store*> create(const QString &mimeType);

    virtual ~Store();

    QString dbName() const;
    OpenMode openMode() const;
    void setOpenMode(OpenMode openMode);

    virtual bool index(qint64 id, const QByteArray &serializedIndex);
    virtual bool removeItem(qint64 id);
    virtual bool removeCollection(qint64 id);
    virtual bool move(const qint64, qint64 srcCollection, qint64 destCollection);

    ResultIterator search(const QByteArray &serializedQuery);

    bool commit();

protected:
    explicit Store();

    void setDbName(const QString &name);

private:
    StorePrivate * const d;
};

}
}

#endif
