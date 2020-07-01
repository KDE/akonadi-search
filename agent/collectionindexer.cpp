/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include <xapian.h>

#include "collectionindexer.h"
#include "xapiandocument.h"

#include <QString>
#include <QStringList>
#include <AkonadiCore/collectionidentificationattribute.h>
#include <AkonadiCore/AttributeFactory>

#include "akonadi_indexer_agent_debug.h"

CollectionIndexer::CollectionIndexer(const QString &path)
{
    Akonadi::AttributeFactory::registerAttribute<Akonadi::CollectionIdentificationAttribute>();

    try {
        m_db = new Xapian::WritableDatabase(path.toUtf8().constData(), Xapian::DB_CREATE_OR_OPEN);
    } catch (const Xapian::DatabaseCorruptError &err) {
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << "Database Corrupted - What did you do?";
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << err.get_error_string();
        m_db = nullptr;
    } catch (const Xapian::Error &e) {
        qCCritical(AKONADI_INDEXER_AGENT_LOG) << QString::fromStdString(e.get_type()) << QString::fromStdString(e.get_description());
        m_db = nullptr;
    }
}

CollectionIndexer::~CollectionIndexer()
{
    commit();
    delete m_db;
}

static QByteArray getPath(const Akonadi::Collection &collection)
{
    QStringList pathParts;
    pathParts << collection.displayName();
    Akonadi::Collection col = collection;
    while (col.parentCollection().isValid() && (col.parentCollection() != Akonadi::Collection::root())) {
        col = col.parentCollection();
        pathParts.prepend(col.displayName());
    }
    return "/" + pathParts.join(QLatin1Char('/')).toUtf8();
}

void CollectionIndexer::index(const Akonadi::Collection &collection)
{
    if (!m_db) {
        return;
    }
    //qCDebug(AKONADI_INDEXER_AGENT_LOG) << "Indexing " << collection.id() << collection.displayName() << collection.name();

    try {
        Xapian::Document doc;
        Xapian::TermGenerator gen;
        gen.set_document(doc);
        gen.set_database(*m_db);

        gen.index_text_without_positions(collection.displayName().toUtf8().constData());
        gen.index_text_without_positions(collection.displayName().toUtf8().constData(), 1, "N");

        //We index with positions so we can do phrase searches (required for exact matches)
        {
            const QByteArray path = getPath(collection);
            gen.index_text(path.constData(), 1, "P");
            const QByteArray term = "A" + path;
            doc.add_term(term.constData());
        }

        Akonadi::Collection::Id colId = collection.parentCollection().id();
        const QByteArray term = 'C' + QByteArray::number(colId);
        doc.add_boolean_term(term.constData());

        QByteArray ns;
        if (const Akonadi::CollectionIdentificationAttribute *folderAttribute = collection.attribute<Akonadi::CollectionIdentificationAttribute>()) {
            if (!folderAttribute->collectionNamespace().isEmpty()) {
                ns = folderAttribute->collectionNamespace();
            }
            if (!folderAttribute->identifier().isEmpty()) {
                const QByteArray term = "ID" + folderAttribute->identifier();
                doc.add_boolean_term(term.constData());
            }
        }
        {
            //We must add the term also with an empty namespace, so we can search for that as well
            const QByteArray term = "NS" + ns;
            doc.add_boolean_term(term.constData());
        }
        const QStringList contentMimeTypes = collection.contentMimeTypes();
        for (const QString &mt : contentMimeTypes) {
            const QByteArray term = "M" + mt.toUtf8();
            doc.add_boolean_term(term.constData());
        }

        m_db->replace_document(collection.id(), doc);
    } catch (const Xapian::Error &e) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Xapian error in indexer:" << e.get_msg().c_str();
    }
}

void CollectionIndexer::change(const Akonadi::Collection &col)
{
    index(col);
}

void CollectionIndexer::remove(const Akonadi::Collection &col)
{
    if (!m_db) {
        return;
    }

    //Remove collection
    try {
        m_db->delete_document(col.id());
    } catch (const Xapian::Error &e) {
        qCWarning(AKONADI_INDEXER_AGENT_LOG) << "Xapian error in indexer:" << e.get_msg().c_str();
    }

    //Remove subcollections
    try {
        Xapian::Query query('C' + QString::number(col.id()).toStdString());
        Xapian::Enquire enquire(*m_db);
        enquire.set_query(query);

        Xapian::MSet mset = enquire.get_mset(0, m_db->get_doccount());
        Xapian::MSetIterator end = mset.end();
        for (Xapian::MSetIterator it = mset.begin(); it != end; ++it) {
            const qint64 id = *it;
            remove(Akonadi::Collection(id));
        }
    } catch (const Xapian::DocNotFoundError &) {
        return;
    }
}

void CollectionIndexer::move(const Akonadi::Collection &collection, const Akonadi::Collection &from, const Akonadi::Collection &to)
{
    Q_UNUSED(from);
    Q_UNUSED(to);
    index(collection);
}

void CollectionIndexer::commit()
{
    if (m_db) {
        m_db->commit();
    }
}
