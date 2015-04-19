/*
 * Copyright 2014 Christian Mollekopf <mollekopf@kolabsys.com>
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
#include "collectionindexer.h"

#include <QString>
#include <QStringList>
#include <xapian/error.h>
#include <xapian/database.h>
#include <xapian/query.h>
#include <xapian/enquire.h>
#include <xapian/termgenerator.h>
#include <AkonadiCore/collectionidentificationattribute.h>
#include <AkonadiCore/AttributeFactory>
#include <xapiandocument.h>
#include <QDebug>

CollectionIndexer::CollectionIndexer(const QString &path)
{
    Akonadi::AttributeFactory::registerAttribute<CollectionIdentificationAttribute>();

    try {
        m_db = new Xapian::WritableDatabase(path.toUtf8().constData(), Xapian::DB_CREATE_OR_OPEN);
    } catch (const Xapian::DatabaseCorruptError &err) {
        qCritical() << "Database Corrupted - What did you do?";
        qCritical() << err.get_error_string();
        m_db = 0;
    } catch (const Xapian::Error &e) {
        qCritical() << QString::fromStdString(e.get_type()) << QString::fromStdString(e.get_description());
        m_db = 0;
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
    return "/" + pathParts.join(QLatin1String("/")).toUtf8();
}

void CollectionIndexer::index(const Akonadi::Collection &collection)
{
    if (!m_db) {
        return;
    }
    //qDebug() << "Indexing " << collection.id() << collection.displayName() << collection.name();

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

        Akonadi::Entity::Id colId = collection.parentCollection().id();
        const QByteArray term = 'C' + QByteArray::number(colId);
        doc.add_boolean_term(term.constData());

        QByteArray ns;
        if (CollectionIdentificationAttribute *folderAttribute = collection.attribute<CollectionIdentificationAttribute>()) {
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
        Q_FOREACH (const QString &mt, collection.contentMimeTypes()) {
            const QByteArray term = "M" + mt.toUtf8();
            doc.add_boolean_term(term.constData());
        }

        m_db->replace_document(collection.id(), doc);
    } catch (const Xapian::Error &e) {
        qWarning() << "Xapian error in indexer:" << e.get_msg().c_str();
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
        qWarning() << "Xapian error in indexer:" << e.get_msg().c_str();
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

void CollectionIndexer::move(const Akonadi::Collection &collection,
                             const Akonadi::Collection &from,
                             const Akonadi::Collection &to)
{
    index(collection);
}

void CollectionIndexer::commit()
{
    if (m_db) {
        m_db->commit();
    }
}
