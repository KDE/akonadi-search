/*
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#ifndef CONTACTINDEXER_H
#define CONTACTINDEXER_H

#include "abstractindexer.h"
#include "xapiandatabase.h"

class ContactIndexer : public AbstractIndexer
{
public:
    explicit ContactIndexer(const QString &path);
    ~ContactIndexer();

    QStringList mimeTypes() const override;

    void index(const Akonadi::Item &item) override;
    void remove(const Akonadi::Item &item) override;
    void remove(const Akonadi::Collection &item) override;

    void commit() override;

    void move(Akonadi::Item::Id itemId, Akonadi::Collection::Id from, Akonadi::Collection::Id to) override;
private:
    bool indexContact(const Akonadi::Item &item);
    void indexContactGroup(const Akonadi::Item &item);

    Akonadi::Search::XapianDatabase *m_db = nullptr;
};

#endif // CONTACTINDEXER_H
