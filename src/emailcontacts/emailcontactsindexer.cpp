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

#include <xapian.h>

#include "emailcontactsindexer.h"
#include "akonadisearch_debug.h"
#include "xapiandocument.h"
#include "utils.h"

#include <AkonadiCore/Item>
#include <AkonadiCore/SearchQuery>

#include <KMime/Message>
#include <KEmailAddress>

#include <QDataStream>

#include <hash_map>

using namespace Akonadi::Search;

QStringList EmailContactsIndexer::mimeTypes()
{
    return { EmailContactsMimeType() };
}

bool EmailContactsIndexer::doIndex(const Item &item, const Collection &parent, QDataStream &stream)
{
    Xapian::Document xapDoc;
    if (!item.hasPayload()) {
        qCWarning(AKONADISEARCH_LOG) << "Item" << item.id() << "does not contain the requested payload: No payload set";
        return false;
    }

    KMime::Message::Ptr msg;
    try {
        msg = item.payload<KMime::Message::Ptr>();
    } catch (const Akonadi::PayloadException &e) {
        return false;
    }

    if (const auto to = msg->to(false)) {
        insert(to->mailboxes(), stream);
    }
    if (const auto from = msg->from(false)) {
        insert(from->mailboxes(), stream);
    }
    if (const auto cc = msg->cc(false)) {
        insert(cc->mailboxes(), stream);
    }
    if (const auto bcc = msg->bcc(false)) {
        insert(bcc->mailboxes(), stream);
    }
    if (const auto replyTo = msg->replyTo(false)) {
        insert(replyTo->mailboxes(), stream);
    }

    return true;
}


namespace
{
// Does some extra stuff such as lower casing the email, removing all quotes
// and removing extra spaces
// TODO: Move this into KMime?
// TODO: If name is all upper/lower then try to captialize it?
QString prettyAddress(const KMime::Types::Mailbox &mbox)
{
    const QString name = mbox.name().simplified();
    const QByteArray email = mbox.address().simplified().toLower();
    return KEmailAddress::normalizedAddress(name, QString::fromUtf8(email));
}
}

void EmailContactsIndexer::insert(const KMime::Types::Mailbox::List &list, QDataStream &stream)
{
    for (const KMime::Types::Mailbox &mbox : list) {
        const QString pa = prettyAddress(mbox);
        Xapian::Document doc;
        std::string pretty(pa.toUtf8().constData());
        doc.set_data(pretty);

        Xapian::TermGenerator termGen;
        termGen.set_document(doc);
        termGen.index_text(pretty);

        doc.add_term(mbox.address().data());
        stream << qint64(qHash(pa)) << doc;
    }
}

