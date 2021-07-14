/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "contactindexer.h"
#include "akonadi_indexer_agent_debug.h"
#include "xapiandocument.h"

#include <Collection>
#include <KContacts/Addressee>
#include <KContacts/ContactGroup>

ContactIndexer::ContactIndexer(const QString &path)
    : AbstractIndexer()
{
    try {
        m_db = new Akonadi::Search::XapianDatabase(path, true);
    } catch (const Xapian::DatabaseCorruptError &err) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Database Corrupted - What did you do?";
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << err.get_error_string();
        m_db = nullptr;
    } catch (const Xapian::Error &e) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << QString::fromStdString(e.get_type()) << QString::fromStdString(e.get_description());
        m_db = nullptr;
    }
}

ContactIndexer::~ContactIndexer()
{
    commit();
    delete m_db;
}

QStringList ContactIndexer::mimeTypes() const
{
    return QStringList() << KContacts::Addressee::mimeType() << KContacts::ContactGroup::mimeType();
}

bool ContactIndexer::indexContact(const Akonadi::Item &item)
{
    if (!m_db) {
        return false;
    }
    KContacts::Addressee addressee;
    try {
        addressee = item.payload<KContacts::Addressee>();
    } catch (const Akonadi::PayloadException &) {
        return false;
    }

    Akonadi::Search::XapianDocument doc;

    QString name;
    if (!addressee.formattedName().isEmpty()) {
        name = addressee.formattedName();
    } else if (!addressee.assembledName().isEmpty()) {
        name = addressee.assembledName();
    } else {
        name = addressee.name();
    }

    qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Indexing" << name << addressee.nickName();

    doc.indexText(name);
    doc.indexText(addressee.nickName());
    doc.indexText(addressee.uid(), QStringLiteral("UID"));

    doc.indexText(name, QStringLiteral("NA"));
    doc.indexText(addressee.nickName(), QStringLiteral("NI"));

    const QStringList lstEmails = addressee.emails();
    for (const QString &email : lstEmails) {
        doc.addTerm(email);
        doc.indexText(email);
    }

    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Akonadi::Search::ContactIndexer::index", "Item does not have a valid parent collection");

    const Akonadi::Collection::Id colId = item.parentCollection().id();
    doc.addBoolTerm(colId, QStringLiteral("C"));

    if (addressee.birthday().isValid()) {
        const QString julianDay = QString::number(addressee.birthday().date().toJulianDay());
        doc.addValue(0, julianDay);
    }
    // TODO index anniversary ?

    m_db->replaceDocument(item.id(), doc);
    return true;
}

void ContactIndexer::indexContactGroup(const Akonadi::Item &item)
{
    if (!m_db) {
        return;
    }
    KContacts::ContactGroup group;
    try {
        group = item.payload<KContacts::ContactGroup>();
    } catch (const Akonadi::PayloadException &) {
        return;
    }

    Akonadi::Search::XapianDocument doc;

    const QString name = group.name();
    doc.indexText(name);
    doc.indexText(name, QStringLiteral("NA"));

    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Akonadi::Search::ContactIndexer::index", "Item does not have a valid parent collection");

    const Akonadi::Collection::Id colId = item.parentCollection().id();
    doc.addBoolTerm(colId, QStringLiteral("C"));
    m_db->replaceDocument(item.id(), doc);
}

void ContactIndexer::index(const Akonadi::Item &item)
{
    if (!indexContact(item)) {
        indexContactGroup(item);
    }
}

void ContactIndexer::remove(const Akonadi::Item &item)
{
    if (m_db) {
        m_db->deleteDocument(item.id());
    }
}

void ContactIndexer::remove(const Akonadi::Collection &collection)
{
    if (!m_db) {
        return;
    }
    try {
        Xapian::Database *db = m_db->db();
        Xapian::Query query('C' + QString::number(collection.id()).toStdString());
        Xapian::Enquire enquire(*db);
        enquire.set_query(query);

        Xapian::MSet mset = enquire.get_mset(0, db->get_doccount());
        Xapian::MSetIterator end(mset.end());
        for (Xapian::MSetIterator it = mset.begin(); it != end; ++it) {
            const qint64 id = *it;
            remove(Akonadi::Item(id));
        }
    } catch (const Xapian::DocNotFoundError &) {
        return;
    }
}

void ContactIndexer::commit()
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

void ContactIndexer::move(Akonadi::Item::Id itemId, Akonadi::Collection::Id from, Akonadi::Collection::Id to)
{
    if (!m_db) {
        return;
    }

    Akonadi::Search::XapianDocument doc;
    try {
        doc = m_db->document(itemId);
    } catch (const Xapian::DocNotFoundError &) {
        return;
    }

    const QByteArray ft = 'C' + QByteArray::number(from);
    const QByteArray tt = 'C' + QByteArray::number(to);

    doc.removeTermStartsWith(ft.data());
    doc.addBoolTerm(QString::fromLatin1(tt.data()));
    m_db->replaceDocument(doc.doc().get_docid(), doc);
}
