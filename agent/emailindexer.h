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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef EMAILINDEXER_H
#define EMAILINDEXER_H

#include "abstractindexer.h"

#include <xapian.h>

#include <KMime/Message>
#include <Akonadi/KMime/MessageStatus>

class EmailIndexer: public AbstractIndexer
{
public:
    /**
     * You must provide the path where the indexed information
     * should be stored
     */
    EmailIndexer(const QString &path, const QString &contactDbPath);
    ~EmailIndexer();

    QStringList mimeTypes() const override;

    void index(const Akonadi::Item &item) override;
    void updateFlags(const Akonadi::Item &item, const QSet<QByteArray> &added,
                     const QSet<QByteArray> &removed) override;
    void remove(const Akonadi::Item &item) override;
    void remove(const Akonadi::Collection &item) override;
    void move(Akonadi::Item::Id itemId,
              Akonadi::Collection::Id from,
              Akonadi::Collection::Id to) override;

    void commit() override;

private:
    Xapian::WritableDatabase *m_db = nullptr;
    Xapian::Document *m_doc = nullptr;
    Xapian::TermGenerator *m_termGen = nullptr;

    Xapian::WritableDatabase *m_contactDb = nullptr;

    void toggleFlag(Xapian::Document &doc, const char *remove, const char *add);

    void process(const KMime::Message::Ptr &msg);
    void processPart(KMime::Content *content, KMime::Content *mainContent);
    void processMessageStatus(const Akonadi::MessageStatus &status);

    void insert(const QByteArray &key, KMime::Headers::Base *base);
    void insert(const QByteArray &key, KMime::Headers::Generics::MailboxList *mlist);
    void insert(const QByteArray &key, KMime::Headers::Generics::AddressList *alist);
    void insert(const QByteArray &key, const KMime::Types::Mailbox::List &list);

    void insertBool(char key, bool value);
};

#endif // EMAILINDEXER_H
