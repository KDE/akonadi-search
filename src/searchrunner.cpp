/*
 * Copyright (C) 2018  Daniel Vrátil <dvratil@kde.org>
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

#include "searchrunner.h"
#include "querymapper.h"
#include "store.h"

#include <AkonadiCore/SearchQuery>

#include <QFutureWatcher>
#include <QtConcurrentRun>

Q_DECLARE_METATYPE(Akonadi::Search::ResultIterator)

namespace Akonadi {
namespace Search {
class Q_DECL_HIDDEN SearchRunner::Private {
public:
    Private(const SearchQuery &query, const QString &mimeType)
        : query(query)
        , mimeType(mimeType)
    {}

    SearchQuery query;
    QString mimeType;
    QFutureWatcher<ResultIterator> future;
    uint limit = 0;
    bool autodelete = false;
};
}
}

using namespace Akonadi::Search;

SearchRunner::SearchRunner(const SearchQuery &query, const QString &mimeType, QObject *parent)
    : QObject(parent)
    , d(new Private(query, mimeType))
{
}

SearchRunner::~SearchRunner()
{
}


void SearchRunner::setAutoDelete(bool autodelete)
{
    d->autodelete = autodelete;
}

bool SearchRunner::autoDelete() const
{
    return d->autodelete;
}

void SearchRunner::setLimit(uint limit)
{
    d->limit = limit;
}

uint SearchRunner::limit() const
{
    return d->limit;
}

void SearchRunner::start()
{
    connect(&d->future, &QFutureWatcher<ResultIterator>::finished,
            this, [this]() {
                Q_EMIT finished(d->future.result());
                if (d->autodelete) {
                    deleteLater();
                }
            });
    d->future.setFuture(QtConcurrent::run([this]() {
        QScopedPointer<QueryMapper> mapper(QueryMapper::create(d->mimeType));
        const auto query = mapper->map(d->query);

        QScopedPointer<Store> store(Store::create(d->mimeType));
        return store->search(query, d->limit);
    }));
}
