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

#ifndef AKONADISEARCH_CONTACTCOMPLETER_H
#define AKONADISEARCH_CONTACTCOMPLETER_H

#include <QString>
#include <QObject>
#include "akonadisearch_export.h"

namespace Akonadi
{
namespace Search
{

class AKONADISEARCH_EXPORT ContactCompleter : public QObject
{
    Q_OBJECT
public:
    explicit ContactCompleter(const QString &query, int limit = 10);
    ~ContactCompleter() override;

    void start();

    void setAutoDelete(bool autodelete);
    bool autoDelete() const;
Q_SIGNALS:
    void finished(const QStringList &results);

private:
    class Private;
    QScopedPointer<Private> const d;
};

}
}
#endif // AKONADISEARCH_EXPORT
