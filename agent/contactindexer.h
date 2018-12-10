/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef CONTACTINDEXER_H
#define CONTACTINDEXER_H

#include "abstractindexer.h"
#include "xapiandatabase.h"

class ContactIndexer: public AbstractIndexer
{
public:
    explicit ContactIndexer(const QString &path);
    ~ContactIndexer();

    QStringList mimeTypes() const override;

    void index(const Akonadi::Item &item) override;
    void remove(const Akonadi::Item &item) override;
    void remove(const Akonadi::Collection &item) override;

    void commit() override;

    void move(Akonadi::Item::Id itemId,
              Akonadi::Collection::Id from,
              Akonadi::Collection::Id to) override;
private:
    bool indexContact(const Akonadi::Item &item);
    void indexContactGroup(const Akonadi::Item &item);

    Akonadi::Search::XapianDatabase *m_db = nullptr;
};

#endif // CONTACTINDEXER_H
