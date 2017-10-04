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

#ifndef AKONADISEARCH_STORETEST_H_
#define AKONADISEARCH_STORETEST_H_

#include <QObject>
#include <QVector>

namespace Akonadi {
class Item;
namespace Search {
class Store;
class QueryMapper;
}
}

class StoreTest : public QObject
{
    Q_OBJECT

private:
    void indexContacts();
    void indexEmails();
    void indexIncidences();
    void indexNotes();
    void indexItems(const QVector<Akonadi::Item> &items);

    void testStore();
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase(); // cleanup after each testrun

    void testContactStore_data();
    void testContactStore();

    void testEmailStore_data();
    void testEmailStore();

    void testIncidenceStore_data();
    void testIncidenceStore();

    void testNoteStore_data();
    void testNoteStore();
};

#endif
