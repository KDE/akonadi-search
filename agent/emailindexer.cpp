/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "emailindexer.h"
#include "akonadi_indexer_agent_debug.h"
#include <Akonadi/Collection>
#include <Akonadi/KMime/MessageFlags>

#include <KEmailAddress>
#include <QTextDocument>

EmailIndexer::EmailIndexer(const QString &path, const QString &contactDbPath)
    : AbstractIndexer()
{
    try {
        m_db = new Xapian::WritableDatabase(path.toStdString(), Xapian::DB_CREATE_OR_OPEN);
    } catch (const Xapian::DatabaseCorruptError &err) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Database Corrupted - What did you do?";
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << err.get_error_string();
        m_db = nullptr;
    } catch (const Xapian::Error &e) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << QString::fromStdString(e.get_type()) << QString::fromStdString(e.get_description());
        m_db = nullptr;
    }

    try {
        m_contactDb = new Xapian::WritableDatabase(contactDbPath.toStdString(), Xapian::DB_CREATE_OR_OPEN);
    } catch (const Xapian::DatabaseCorruptError &err) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Database Corrupted - What did you do?";
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << err.get_error_string();
        m_contactDb = nullptr;
    } catch (const Xapian::Error &e) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << QString::fromStdString(e.get_type()) << QString::fromStdString(e.get_description());
        m_contactDb = nullptr;
    }
}

EmailIndexer::~EmailIndexer()
{
    commit();
    delete m_db;
    delete m_contactDb;
}

QStringList EmailIndexer::mimeTypes() const
{
    return QStringList() << KMime::Message::mimeType();
}

void EmailIndexer::index(const Akonadi::Item &item)
{
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Indexing item" << item.id();
    if (!m_db) {
        return;
    }
    Akonadi::MessageStatus status;
    status.setStatusFromFlags(item.flags());
    if (status.isSpam()) {
        return;
    }

    KMime::Message::Ptr msg;
    try {
        msg = item.payload<KMime::Message::Ptr>();
    } catch (const Akonadi::PayloadException &) {
        return;
    }

    m_doc = new Xapian::Document();
    m_termGen = new Xapian::TermGenerator();
    m_termGen->set_document(*m_doc);
    m_termGen->set_database(*m_db);

    processMessageStatus(status);
    process(msg);

    // Size
    m_doc->add_value(1, QString::number(item.size()).toStdString());

    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Akonadi::Search::EmailIndexer::index", "Item does not have a valid parent collection");

    const Akonadi::Collection::Id colId = item.parentCollection().id();
    const QByteArray term = 'C' + QByteArray::number(colId);
    m_doc->add_boolean_term(term.data());

    m_db->replace_document(item.id(), *m_doc);

    delete m_doc;
    delete m_termGen;

    m_doc = nullptr;
    m_termGen = nullptr;
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "DONE Indexing item" << item.id();
}

void EmailIndexer::insert(const QByteArray &key, KMime::Headers::Base *unstructured)
{
    if (unstructured) {
        m_termGen->index_text_without_positions(unstructured->asUnicodeString().toStdString(), 1, key.data());
    }
}

void EmailIndexer::insert(const QByteArray &key, KMime::Headers::Generics::MailboxList *mlist)
{
    if (mlist) {
        insert(key, mlist->mailboxes());
    }
}

void EmailIndexer::insert(const QByteArray &key, KMime::Headers::Generics::AddressList *alist)
{
    if (alist) {
        insert(key, alist->mailboxes());
    }
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

// Add once with a prefix and once without
void EmailIndexer::insert(const QByteArray &key, const KMime::Types::Mailbox::List &list)
{
    if (!m_contactDb) {
        return;
    }
    for (const KMime::Types::Mailbox &mbox : list) {
        const auto name(mbox.name().toStdString());
        m_termGen->index_text_without_positions(name, 1, key.data());
        m_termGen->index_text_without_positions(name, 1);
        m_termGen->index_text_without_positions(mbox.address().data(), 1, key.data());
        m_termGen->index_text_without_positions(mbox.address().data(), 1);

        m_doc->add_term(QByteArray(key + mbox.address()).data());
        m_doc->add_term(mbox.address().data());

        //
        // Add emails for email auto-completion
        //
        const auto pa = prettyAddress(mbox);
        const auto id = qHash(pa);
        try {
            const auto doc = m_contactDb->get_document(id);
            Q_UNUSED(doc);
            continue;
        } catch (const Xapian::DocNotFoundError &) {
            Xapian::Document doc;
            const auto pretty(pa.toStdString());
            doc.set_data(pretty);

            Xapian::TermGenerator termGen;
            termGen.set_document(doc);
            termGen.index_text(pretty);

            doc.add_term(mbox.address().data());
            m_contactDb->replace_document(id, doc);
        }
    }
}

// FIXME: Only index properties that are actually searched!
void EmailIndexer::process(const KMime::Message::Ptr &msg)
{
    //
    // Process Headers
    // (Give the subject a higher priority)
    KMime::Headers::Subject *subject = msg->subject(false);
    if (subject) {
        const std::string str(subject->asUnicodeString().toStdString());
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Indexing" << str.c_str();
        m_termGen->index_text_without_positions(str, 1, "SU");
        m_termGen->index_text_without_positions(str, 100);
        m_doc->set_data(str);
    }

    KMime::Headers::Date *date = msg->date(false);
    if (date) {
        const QString str = QString::number(date->dateTime().toSecsSinceEpoch());
        m_doc->add_value(0, str.toStdString());
        const QString julianDay = QString::number(date->dateTime().date().toJulianDay());
        m_doc->add_value(2, julianDay.toStdString());
    }

    insert("F", msg->from(false));
    insert("T", msg->to(false));
    insert("CC", msg->cc(false));
    insert("BC", msg->bcc(false));
    insert("O", msg->organization(false));
    insert("RT", msg->replyTo(false));
    insert("RF", msg->headerByType("Resent-From"));
    insert("LI", msg->headerByType("List-Id"));
    insert("XL", msg->headerByType("X-Loop"));
    insert("XML", msg->headerByType("X-Mailing-List"));
    insert("XSF", msg->headerByType("X-Spam-Flag"));

    //
    // Process Plain Text Content
    //

    // Index all headers
    m_termGen->index_text_without_positions(std::string(msg->head().constData()), 1, "HE");

    KMime::Content *mainBody = msg->mainBodyPart("text/plain");
    if (mainBody) {
        const std::string text(mainBody->decodedText().toStdString());
        m_termGen->index_text_without_positions(text);
        m_termGen->index_text_without_positions(text, 1, "BO");
    } else {
        processPart(msg.data(), nullptr);
    }
}

void EmailIndexer::processPart(KMime::Content *content, KMime::Content *mainContent)
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
                processPart(c, mainContent);
            }
        }

        // Only get HTML content, if no plain text content
        if (!mainContent && type->isHTMLText()) {
            QTextDocument doc;
            doc.setHtml(content->decodedText());

            const std::string text(doc.toPlainText().toStdString());
            m_termGen->index_text_without_positions(text);
        }
    }

    // FIXME: Handle attachments?
}

void EmailIndexer::processMessageStatus(Akonadi::MessageStatus status)
{
    insertBool('R', status.isRead());
    insertBool('A', status.hasAttachment());
    insertBool('I', status.isImportant());
    insertBool('W', status.isWatched());
    insertBool('T', status.isToAct());
    insertBool('D', status.isDeleted());
    insertBool('S', status.isSpam());
    insertBool('E', status.isReplied());
    insertBool('G', status.isIgnored());
    insertBool('F', status.isForwarded());
    insertBool('N', status.isSent());
    insertBool('Q', status.isQueued());
    insertBool('H', status.isHam());
    insertBool('C', status.isEncrypted());
    insertBool('V', status.hasInvitation());
}

void EmailIndexer::insertBool(char key, bool value)
{
    QByteArray term("B");
    if (value) {
        term.append(key);
    } else {
        term.append('N');
        term.append(key);
    }

    m_doc->add_boolean_term(term.data());
}

void EmailIndexer::toggleFlag(Xapian::Document &doc, const char *remove, const char *add)
{
    try {
        doc.remove_term(remove);
    } catch (const Xapian::InvalidArgumentError &e) {
        // The previous flag state was not indexed, continue
    }

    doc.add_term(add);
}

void EmailIndexer::updateFlags(const Akonadi::Item &item, const QSet<QByteArray> &added, const QSet<QByteArray> &removed)
{
    if (!m_db) {
        return;
    }
    Xapian::Document doc;
    try {
        doc = m_db->get_document(item.id());
    } catch (const Xapian::DocNotFoundError &) {
        return;
    }

    for (const QByteArray &flag : removed) {
        if (flag == Akonadi::MessageFlags::Seen) {
            toggleFlag(doc, "BR", "BNR");
        } else if (flag == Akonadi::MessageFlags::Flagged) {
            toggleFlag(doc, "BI", "BNI");
        } else if (flag == Akonadi::MessageFlags::Watched) {
            toggleFlag(doc, "BW", "BNW");
        }
    }

    for (const QByteArray &flag : added) {
        if (flag == Akonadi::MessageFlags::Seen) {
            toggleFlag(doc, "BNR", "BR");
        } else if (flag == Akonadi::MessageFlags::Flagged) {
            toggleFlag(doc, "BNI", "BI");
        } else if (flag == Akonadi::MessageFlags::Watched) {
            toggleFlag(doc, "BNW", "BW");
        }
    }

    m_db->replace_document(doc.get_docid(), doc);
}

void EmailIndexer::remove(const Akonadi::Item &item)
{
    if (!m_db) {
        return;
    }
    try {
        m_db->delete_document(item.id());
        // TODO remove contacts from contact db?
    } catch (const Xapian::DocNotFoundError &) {
        return;
    }
}

void EmailIndexer::remove(const Akonadi::Collection &collection)
{
    if (!m_db) {
        return;
    }
    try {
        Xapian::Query query('C' + QString::number(collection.id()).toStdString());
        Xapian::Enquire enquire(*m_db);
        enquire.set_query(query);

        Xapian::MSet mset = enquire.get_mset(0, m_db->get_doccount());
        Xapian::MSetIterator end = mset.end();
        for (Xapian::MSetIterator it = mset.begin(); it != end; ++it) {
            const qint64 id = *it;
            remove(Akonadi::Item(id));
        }
    } catch (const Xapian::DocNotFoundError &) {
        return;
    }
}

void EmailIndexer::move(Akonadi::Item::Id itemId, Akonadi::Collection::Id from, Akonadi::Collection::Id to)
{
    if (!m_db) {
        return;
    }
    Xapian::Document doc;
    try {
        doc = m_db->get_document(itemId);
    } catch (const Xapian::DocNotFoundError &) {
        return;
    }

    const QByteArray ft = 'C' + QByteArray::number(from);
    const QByteArray tt = 'C' + QByteArray::number(to);

    doc.remove_term(ft.data());
    doc.add_boolean_term(tt.data());
    m_db->replace_document(doc.get_docid(), doc);
}

void EmailIndexer::commit()
{
    if (m_db) {
        try {
            m_db->commit();
        } catch (const Xapian::Error &err) {
            qCWarning(AKONADI_INDEXER_AGENT_LOG) << err.get_error_string();
        }
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Xapian Committed";
    }

    if (m_contactDb) {
        try {
            m_contactDb->commit();
        } catch (const Xapian::Error &err) {
            qCWarning(AKONADI_INDEXER_AGENT_LOG) << err.get_error_string();
        }
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Xapian Committed";
    }
}
