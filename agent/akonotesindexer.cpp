/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2025 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "akonotesindexer.h"
using namespace Qt::Literals::StringLiterals;

#include "akonadi_indexer_agent_debug.h"
#ifdef HAS_HTMLPARSER
#include <lib.rs.h>
#else
#include <QTextDocument>
#endif

AkonotesIndexer::AkonotesIndexer(const QString &path)

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
}

AkonotesIndexer::~AkonotesIndexer()
{
    commit();
    delete m_db;
}

QStringList AkonotesIndexer::mimeTypes() const
{
    return {u"text/x-vnd.akonadi.note"_s};
}

void AkonotesIndexer::index(const Akonadi::Item &item)
{
    if (!m_db) {
        return;
    }
    std::shared_ptr<KMime::Message> msg;
    try {
        msg = item.payload<std::shared_ptr<KMime::Message>>();
    } catch (const Akonadi::PayloadException &) {
        return;
    }
    m_doc = new Xapian::Document();
    m_termGen = new Xapian::TermGenerator();
    m_termGen->set_document(*m_doc);
    m_termGen->set_database(*m_db);

    process(msg);

    const Akonadi::Collection::Id colId = item.parentCollection().id();
    const QByteArray term = 'C' + QByteArray::number(colId);
    m_doc->add_boolean_term(term.data());

    m_db->replace_document(item.id(), *m_doc);

    delete m_doc;
    delete m_termGen;

    m_doc = nullptr;
    m_termGen = nullptr;
}

void AkonotesIndexer::process(const std::shared_ptr<KMime::Message> &msg)
{
    //
    // Process Headers
    // (Give the subject a higher priority)
    KMime::Headers::Subject *subject = msg->subject(KMime::DontCreate);
    if (subject) {
        const std::string str(normalizeString(subject->asUnicodeString()).toStdString());
        qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Indexing" << str.c_str();
        m_termGen->index_text_without_positions(str, 1, "SU");
        m_termGen->index_text_without_positions(str, 100);
        m_doc->set_data(str);
    }

    KMime::Content *mainBody = msg->mainBodyPart("text/plain");
    if (mainBody) {
        const std::string text(normalizeString(mainBody->decodedText()).toStdString());
        m_termGen->index_text_without_positions(text);
        m_termGen->index_text_without_positions(text, 1, "BO");
    } else {
        processPart(msg.get(), nullptr);
    }
}

void AkonotesIndexer::processPart(KMime::Content *content, KMime::Content *mainContent)
{
    if (content == mainContent) {
        return;
    }

    KMime::Headers::ContentType *type = content->contentType(KMime::DontCreate);
    if (type) {
        if (type->isMultipart()) {
            if (type->isSubtype("encrypted")) {
                return;
            }

            const auto contents = content->contents();
            for (KMime::Content *c : contents) {
                processPart(c, mainContent);
            }
        }

        // Only get HTML content, if no plain text content
        if (!mainContent && type->isHTMLText()) {
#ifdef HAS_HTMLPARSER
            const auto html = content->decodedText().toStdString();
            const auto text = std::string(convert_to_text(rust::String(html)));
#else
            QTextDocument doc;
            doc.setHtml(content->decodedText());
            const std::string text(normalizeString(doc.toPlainText()).toStdString());
#endif
            m_termGen->index_text_without_positions(text);
        }
    }
}

void AkonotesIndexer::commit()
{
    if (!m_db) {
        return;
    }

    try {
        m_db->commit();
    } catch (const Xapian::Error &err) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << err.get_error_string();
    }
    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Xapian Committed";
}

void AkonotesIndexer::remove(const Akonadi::Item &item)
{
    if (!m_db) {
        return;
    }
    try {
        m_db->delete_document(item.id());
    } catch (const Xapian::DocNotFoundError &) {
        return;
    }
}

void AkonotesIndexer::remove(const Akonadi::Collection &collection)
{
    if (!m_db) {
        return;
    }
    try {
        const Xapian::Query query('C' + QString::number(collection.id()).toStdString());
        Xapian::Enquire enquire(*m_db);
        enquire.set_query(query);

        Xapian::MSet mset = enquire.get_mset(0, m_db->get_doccount());
        Xapian::MSetIterator end(mset.end());
        for (Xapian::MSetIterator it = mset.begin(); it != end; ++it) {
            const qint64 id = *it;
            remove(Akonadi::Item(id));
        }
    } catch (const Xapian::DocNotFoundError &) {
        return;
    }
}

void AkonotesIndexer::move(Akonadi::Item::Id itemId, Akonadi::Collection::Id from, Akonadi::Collection::Id to)
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
