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
#include "emailquerypropertymapper.h"
#include "xapiandocument.h"
#include "akonadisearch_debug.h"

#include <AkonadiCore/Item>
#include <AkonadiCore/SearchQuery>

#include <KMime/Headers>
#include <KMime/Content>

#include <QTextDocument>

using namespace Akonadi::Search;

QStringList EmailIndexer::mimeTypes()
{
    return { KMime::Message::mimeType() };
}

Xapian::Document EmailIndexer::doIndex(const Akonadi::Item &item)
{
    Akonadi::MessageStatus status;
    status.setStatusFromFlags(item.flags());
    if (status.isSpam()) {
        return {};
    }

    XapianDocument doc;
    processMessageStatus(doc, status);

    KMime::Message::Ptr msg;
    try {
        msg = item.payload<KMime::Message::Ptr>();
    } catch (const Akonadi::PayloadException &e) {
        // It's perfectly possible that we only have flags
        return doc.xapianDocument();
    }

    process(doc, msg);

    doc.addValue(EmailQueryPropertyMapper::instance().valueProperty(Akonadi::EmailSearchTerm::ByteSize), item.size());

    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Akonadi::Search::EmailIndexer::index",
               "Item does not have a valid parent collection");

    Akonadi::Collection::Id colId = item.parentCollection().id();
    doc.addCollectionTerm(colId);

    return doc.xapianDocument();
}

void EmailIndexer::processHeader(XapianDocument &doc, const std::string &key, KMime::Headers::Base *unstructured)
{
    if (unstructured) {
        doc.indexText(unstructured->asUnicodeString(), key);
    }
}

void EmailIndexer::processHeader(XapianDocument &doc, const std::string &key, KMime::Headers::Generics::MailboxList *mlist)
{
    if (mlist) {
        processMailboxes(doc, key, mlist->mailboxes());
    }
}

void EmailIndexer::processHeader(XapianDocument &doc, const std::string &key, KMime::Headers::Generics::AddressList *alist)
{
    if (alist) {
        processMailboxes(doc, key, alist->mailboxes());
    }
}


// Add once with a prefix and once without
void EmailIndexer::processMailboxes(XapianDocument &doc, const std::string &key, const KMime::Types::Mailbox::List &list)
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
    const auto &propMapper = EmailQueryPropertyMapper::instance();

    // Process Headers
    // (Give the subject a higher priority)
    KMime::Headers::Subject *subject = msg->subject(false);
    if (subject) {
        const QString str = subject->asUnicodeString();
        doc.indexText(str, propMapper.prefix(Akonadi::EmailSearchTerm::Subject), 1);
        doc.indexTextWithoutPositions(str, {}, 100);
        doc.setData(str);
    }

    processHeader(doc, propMapper.prefix(Akonadi::EmailSearchTerm::HeaderFrom), msg->from(false));
    processHeader(doc, propMapper.prefix(Akonadi::EmailSearchTerm::HeaderTo), msg->to(false));
    processHeader(doc, propMapper.prefix(Akonadi::EmailSearchTerm::HeaderCC), msg->cc(false));
    processHeader(doc, propMapper.prefix(Akonadi::EmailSearchTerm::HeaderBCC), msg->bcc(false));
    processHeader(doc, propMapper.prefix(Akonadi::EmailSearchTerm::HeaderOrganization), msg->organization(false));
    processHeader(doc, propMapper.prefix(Akonadi::EmailSearchTerm::HeaderReplyTo), msg->replyTo(false));
    processHeader(doc, propMapper.prefix(Akonadi::EmailSearchTerm::HeaderResentFrom), msg->headerByType("Resent-From"));
    processHeader(doc, propMapper.prefix(Akonadi::EmailSearchTerm::HeaderListId), msg->headerByType("List-Id"));
    processHeader(doc, propMapper.prefix(Akonadi::EmailSearchTerm::HeaderXLoop), msg->headerByType("X-Loop"));
    processHeader(doc, propMapper.prefix(Akonadi::EmailSearchTerm::HeaderXMailingList), msg->headerByType("X-Mailing-List"));
    processHeader(doc, propMapper.prefix(Akonadi::EmailSearchTerm::HeaderXSpamFlag), msg->headerByType("X-Spam-Flag"));

    if (auto date = msg->date(false)) {
        doc.addValue(propMapper.valueProperty(Akonadi::EmailSearchTerm::HeaderDate),
                     date->dateTime().toSecsSinceEpoch());
        doc.addValue(propMapper.valueProperty(Akonadi::EmailSearchTerm::HeaderOnlyDate),
                     QDateTime(date->dateTime().date(), {}).toSecsSinceEpoch());
    }

    //
    // Process Plain Text Content
    //

    // TODO: Do we really have any use for this? It only grows the indexes and
    // makes message-wide search useless since it matches all kinds of mess
    //Index all headers
    doc.indexText(QString::fromUtf8(msg->head()), propMapper.prefix(Akonadi::EmailSearchTerm::Headers));

    KMime::Content *mainBody = msg->mainBodyPart("text/plain");
    if (mainBody) {
        const auto text = mainBody->decodedText();
        doc.indexTextWithoutPositions(text, {}, 1);
        doc.indexText(text, propMapper.prefix(Akonadi::EmailSearchTerm::Body), 1);
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
            const auto plainText = textDoc.toPlainText();
            doc.indexTextWithoutPositions(plainText);
            doc.indexText(plainText, EmailQueryPropertyMapper::instance().prefix(Akonadi::EmailSearchTerm::Body));
        }
    }

    // FIXME: Handle attachments?
}

void EmailIndexer::processMessageStatus(XapianDocument &doc, const Akonadi::MessageStatus &status)
{
    const auto &propMapper = EmailQueryPropertyMapper::instance();
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::IsReadFlag), status.isRead());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::HasAttachmentFlag), status.hasAttachment());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::IsImportantFlag), status.isImportant());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::IsWatchedFlag), status.isWatched());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::IsToActFlag), status.isToAct());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::IsDeletedFlag), status.isDeleted());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::IsSpamFlag), status.isSpam());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::IsRepliedFlag), status.isReplied());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::IsIgnoredFlag), status.isIgnored());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::IsForwardedFlag), status.isForwarded());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::IsSentFlag), status.isSent());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::IsQueuedFlag), status.isQueued());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::IsHamFlag), status.isHam());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::IsEncryptedFlag), status.isEncrypted());
    insertBool(doc, propMapper.prefix(EmailQueryPropertyMapper::HasInvitationFlag), status.hasInvitation());
}

void EmailIndexer::insertBool(XapianDocument &doc, const std::string &key, bool value)
{
    QString term = QStringLiteral("B");
    if (value) {
        term.append(QString::fromStdString(key));
    } else {
        term.append(QLatin1Char('N'));
        term.append(QString::fromStdString(key));
    }

    doc.addBoolTerm(term);
}
