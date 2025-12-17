/*
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include <xapian.h>

#include "abstractindexer.h"

#include <Akonadi/MessageStatus>
#include <KMime/Message>

class EmailIndexer : public AbstractIndexer
{
public:
    /**
     * You must provide the path where the indexed information
     * should be stored
     */
    explicit EmailIndexer(const QString &path, const QString &contactDbPath);
    ~EmailIndexer() override;

    [[nodiscard]] QStringList mimeTypes() const override;

    void index(const Akonadi::Item &item) override;
    void updateFlags(const Akonadi::Item &item, const QSet<QByteArray> &added, const QSet<QByteArray> &removed) override;
    void remove(const Akonadi::Item &item) override;
    void remove(const Akonadi::Collection &item) override;
    void move(Akonadi::Item::Id itemId, Akonadi::Collection::Id from, Akonadi::Collection::Id to) override;

    void commit() override;

private:
    Xapian::WritableDatabase *m_db = nullptr;
    Xapian::Document *m_doc = nullptr;
    Xapian::TermGenerator *m_termGen = nullptr;

    Xapian::WritableDatabase *m_contactDb = nullptr;

    void toggleFlag(Xapian::Document &doc, const char *remove, const char *add);

    void process(const std::shared_ptr<KMime::Message> &msg);
    void processPart(KMime::Content *content, KMime::Content *mainContent);
    void processMessageStatus(Akonadi::MessageStatus status);

    void insert(const QByteArray &key, KMime::Headers::Base *base);
    void insert(const QByteArray &key, KMime::Headers::Generics::MailboxList *mlist);
    void insert(const QByteArray &key, KMime::Headers::Generics::AddressList *alist);
    void insert(const QByteArray &key, const QList<KMime::Types::Mailbox> &list);

    void insertBool(char key, bool value);
};
