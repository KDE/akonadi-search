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

#ifndef AKONADISEARCH_EMAILINDEXER_H_
#define AKONADISEARCH_EMAILINDEXER_H_

#include "indexer.h"
#include "xapiantermgenerator.h"

#include <Akonadi/KMime/MessageStatus>
#include <KMime/Message>
#include <KMime/Types>

namespace Akonadi {
namespace Search {

class XapianDocument;
class EmailIndexer : public Indexer
{
public:
    using Indexer::Indexer;

    static QStringList mimeTypes();

    using Indexer::index;
    Xapian::Document index(const Akonadi::Item &item) override;

private:
    void process(XapianDocument &doc, const KMime::Message::Ptr &msg);
    void processMessageStatus(XapianDocument &doc, const Akonadi::MessageStatus &status);
    void processPart(XapianDocument &document, KMime::Content *content, KMime::Content *mainContent);
    void insertBool(XapianDocument &doc, const std::string &key, bool value);
    void processHeader(XapianDocument &doc, const std::string &key, KMime::Headers::Base *unstructured);
    void processHeader(XapianDocument &doc, const std::string &key, KMime::Headers::Generics::MailboxList *mlist);
    void processHeader(XapianDocument &doc, const std::string &key, KMime::Headers::Generics::AddressList *alist);
    void processMailboxes(XapianDocument &doc, const std::string &key, const KMime::Types::Mailbox::List &list);

};

}
}

#endif
