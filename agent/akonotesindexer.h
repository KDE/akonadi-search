/*
 * This file is part of the KDE Akonadi Search Project
 * Copyright (C) 2014-2020 Laurent Montel <montel@kde.org>
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
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef AKONOTESINDEXER_H
#define AKONOTESINDEXER_H

#include <xapian.h>

#include "abstractindexer.h"

#include <KMime/Message>
#include <Collection>
#include <Item>

class AkonotesIndexer : public AbstractIndexer
{
public:
    /**
     * You must provide the path where the indexed information
     * should be stored
     */
    explicit AkonotesIndexer(const QString &path);
    ~AkonotesIndexer();

    QStringList mimeTypes() const override;

    void index(const Akonadi::Item &item) override;
    void commit() override;

    void remove(const Akonadi::Item &item) override;
    void remove(const Akonadi::Collection &collection) override;
    void move(Akonadi::Item::Id itemId, Akonadi::Collection::Id from, Akonadi::Collection::Id to) override;
private:
    void processPart(KMime::Content *content, KMime::Content *mainContent);
    void process(const KMime::Message::Ptr &msg);
    Xapian::WritableDatabase *m_db = nullptr;
    Xapian::Document *m_doc = nullptr;
    Xapian::TermGenerator *m_termGen = nullptr;
};

#endif // AKONOTESINDEXER_H
