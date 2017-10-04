/*
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#include <xapian.h>

#include "emailindexer.h"
#include "xapiandocument.h"

#include <AkonadiCore/Item>

#include <KMime/Headers>
#include <KMime/Content>

#include <QTextDocument>

using namespace Akonadi::Search;

QStringList EmailIndexer::mimeTypes()
{
    return { KMime::Message::mimeType() };
}

Xapian::Document EmailIndexer::index(const Akonadi::Item &item)
{
    Akonadi::MessageStatus status;
    status.setStatusFromFlags(item.flags());
    if (status.isSpam()) {
        return {};
    }

    KMime::Message::Ptr msg;
    try {
        msg = item.payload<KMime::Message::Ptr>();
    } catch (const Akonadi::PayloadException &) {
        return {};
    }

    XapianDocument doc;

    processMessageStatus(doc, status);
    process(doc, msg);

    doc.addValue(1, item.size());

    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Akonadi::Search::EmailIndexer::index",
               "Item does not have a valid parent collection");

    Akonadi::Collection::Id colId = item.parentCollection().id();
    doc.addCollectionTerm(colId);

    return doc.xapianDocument();
}

void EmailIndexer::processHeader(XapianDocument &doc, const QString &key, KMime::Headers::Base *unstructured)
{
    if (unstructured) {
        doc.indexText(unstructured->asUnicodeString(), key);
    }
}

void EmailIndexer::processHeader(XapianDocument &doc, const QString &key, KMime::Headers::Generics::MailboxList *mlist)
{
    if (mlist) {
        processMailboxes(doc, key, mlist->mailboxes());
    }
}

void EmailIndexer::processHeader(XapianDocument &doc, const QString &key, KMime::Headers::Generics::AddressList *alist)
{
    if (alist) {
        processMailboxes(doc, key, alist->mailboxes());
    }
}


// Add once with a prefix and once without
void EmailIndexer::processMailboxes(XapianDocument &doc, const QString &key, const KMime::Types::Mailbox::List &list)
{
    for (const auto &mbox : list) {
        const auto name = mbox.name();
        doc.indexTextWithoutPositions(name, key);
        doc.indexTextWithoutPositions(name);
        const auto address = QString::fromUtf8(mbox.address());
        doc.indexTextWithoutPositions(address, key);
        doc.indexTextWithoutPositions(address);
        doc.indexText(address, key);
        doc.addTerm(address);
        doc.indexText(mbox.prettyAddress(KMime::Types::Mailbox::QuoteNever), key);
    }
}

void EmailIndexer::process(XapianDocument &doc, const KMime::Message::Ptr &msg)
{
    // Process Headers
    // (Give the subject a higher priority)
    KMime::Headers::Subject *subject = msg->subject(false);
    if (subject) {
        const QString str = subject->asUnicodeString();
        doc.indexText(str, QStringLiteral("SU"), 1);
        doc.indexTextWithoutPositions(str, QString(), 100);
        doc.setData(str);
    }

    processHeader(doc, QStringLiteral("F"), msg->from(false));
    processHeader(doc, QStringLiteral("T"), msg->to(false));
    processHeader(doc, QStringLiteral("CC"), msg->cc(false));
    processHeader(doc, QStringLiteral("BC"), msg->bcc(false));
    processHeader(doc, QStringLiteral("O"), msg->organization(false));
    processHeader(doc, QStringLiteral("RT"), msg->replyTo(false));
    processHeader(doc, QStringLiteral("RF"), msg->headerByType("Resent-From"));
    processHeader(doc, QStringLiteral("LI"), msg->headerByType("List-Id"));
    processHeader(doc, QStringLiteral("XL"), msg->headerByType("X-Loop"));
    processHeader(doc, QStringLiteral("XML"), msg->headerByType("X-Mailing-List"));
    processHeader(doc, QStringLiteral("XSF"), msg->headerByType("X-Spam-Flag"));

    if (auto date = msg->date(false)) {
        doc.addValue(0, date->dateTime().toSecsSinceEpoch());
        doc.addValue(2, QDateTime(date->dateTime().date(), {}).toSecsSinceEpoch());
    }

    //
    // Process Plain Text Content
    //

    // TODO: Do we really have any use for this? It only grows the indexes and
    // makes message-wide search useless since it matches all kinds of mess
    //Index all headers
    doc.indexText(QString::fromUtf8(msg->head()), QStringLiteral("HE"));

    KMime::Content *mainBody = msg->mainBodyPart("text/plain");
    if (mainBody) {
        const auto text = mainBody->decodedText();
        doc.indexTextWithoutPositions(text, QString(), 1);
        doc.indexTextWithoutPositions(text, QStringLiteral("BO"), 1);
    } else {
        processPart(doc, msg.data(), nullptr);
    }
}


void EmailIndexer::processPart(XapianDocument &doc, KMime::Content *content, KMime::Content *mainContent)
{
    if (content == mainContent) {
        return;
    }

    KMime::Headers::ContentType *type = content->contentType(false);
    if (type) {
        if (type->isMultipart()) {
            if (type->isSubtype("encrypted")) {
                return;
            }

            for (KMime::Content *c : content->contents()) {
                processPart(doc, c, mainContent);
            }
        }

        // Only get HTML content, if no plain text content
        if (!mainContent && type->isHTMLText()) {
            QTextDocument textDoc;
            textDoc.setHtml(content->decodedText());
            doc.indexTextWithoutPositions(textDoc.toPlainText());
        }
    }

    // FIXME: Handle attachments?
}

void EmailIndexer::processMessageStatus(XapianDocument &doc, const Akonadi::MessageStatus &status)
{
    insertBool(doc, 'R', status.isRead());
    insertBool(doc, 'A', status.hasAttachment());
    insertBool(doc, 'I', status.isImportant());
    insertBool(doc, 'W', status.isWatched());
    insertBool(doc, 'T', status.isToAct());
    insertBool(doc, 'D', status.isDeleted());
    insertBool(doc, 'S', status.isSpam());
    insertBool(doc, 'E', status.isReplied());
    insertBool(doc, 'G', status.isIgnored());
    insertBool(doc, 'F', status.isForwarded());
    insertBool(doc, 'N', status.isSent());
    insertBool(doc, 'Q', status.isQueued());
    insertBool(doc, 'H', status.isHam());
    insertBool(doc, 'C', status.isEncrypted());
    insertBool(doc, 'V', status.hasInvitation());
}

void EmailIndexer::insertBool(XapianDocument &doc, char key, bool value)
{
    QString term = QStringLiteral("B");
    if (value) {
        term.append(QLatin1Char(key));
    } else {
        term.append(QLatin1Char('N'));
        term.append(QLatin1Char(key));
    }

    doc.addBoolTerm(term);
}
