/*
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include "contactcompleter.h"
#include "store.h"
#include "utils.h"
#include "resultiterator.h"
#include "emailcontacts/emailcontactsstore.h"

#include <xapian.h>

#include <QDebug>
#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>

namespace Akonadi {
namespace Search {
class Q_DECL_HIDDEN ContactCompleter::Private {
public:
    Private(const QString &query, int limit)
        : query(query), limit(limit)
    {}

    QString query;
    QFutureWatcher<QStringList> future;
    int limit;
    bool autodelete = true;
};
}
}

using namespace Akonadi::Search;

ContactCompleter::ContactCompleter(const QString &query, int limit)
    : d(new Private(query, limit))
{
}

ContactCompleter::~ContactCompleter()
{}

bool ContactCompleter::autoDelete() const
{
    return d->autodelete;
}

void ContactCompleter::setAutoDelete(bool autodelete)
{
    d->autodelete = autodelete;
}


void ContactCompleter::start()
{
    connect(&d->future, &QFutureWatcher<QStringList>::finished,
            this, [this]() {
                Q_EMIT finished(d->future.future().result());
                if (d->autodelete) {
                    deleteLater();
                }
            });

    d->future.setFuture(QtConcurrent::run([this]() {
        Xapian::QueryParser parser;
        const auto query = parser.parse_query(d->query.toStdString(), Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL).serialise();
        QByteArray serialized(query.c_str(), query.size());

        QScopedPointer<Store> store(Store::create(EmailContactsMimeType()));
        auto iter = store->search(serialized, d->limit);
        QStringList results;
        auto ecStore = static_cast<EmailContactsStore*>(store.get());
        while (iter.next()) {
            results.push_back(ecStore->contactName(iter.id()));
        }

        return results;
    }));
}
