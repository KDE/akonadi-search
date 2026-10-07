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

#include "noteindexer.h"
#include "notequerypropertymapper.h"
#include "xapiandocument.h"
#include "utils.h"

#include <Akonadi/Item>
#include <Akonadi/SearchQuery>

#include <KMime/Message>

#include <QTextDocument>
#include <QDataStream>

using namespace Akonadi::Search;


QStringList NoteIndexer::mimeTypes()
{
    return { QStringLiteral("text/x-vnd.akonadi.note") };
}


bool NoteIndexer::doIndex(const Item &item, const Collection &parent, QDataStream &stream)
{
   std::shared_ptr<KMime::Message> msg;
    try {
        msg = item.payload<std::shared_ptr<KMime::Message>>();
    } catch (const Akonadi::PayloadException &) {
        return false;
    }

    XapianDocument doc(process(msg));

    const auto _parent = parent.isValid() ? parent : item.parentCollection();
    if (!_parent.isValid()) {
        Q_ASSERT_X(_parent.isValid(), "Akonadi::Search::CalenderIndexer::index",
                   "Item does not have a valid parent collection");
        return false;
    }
    doc.addCollectionTerm(_parent.id());

    stream << item.id() << doc.xapianDocument();
    return true;
}

Xapian::Document NoteIndexer::process(const std::shared_ptr<KMime::Message> &note)
{
    const auto &propMapper = NoteQueryPropertyMapper::instance();

    XapianDocument doc;

    // Process Headers
    // (Give the subject a higher priority)
    KMime::Headers::Subject *subject = note->subject(KMime::DontCreate);
    if (subject) {
        const QString str = subject->asUnicodeString();
        doc.indexText(str, propMapper.prefix(Akonadi::EmailSearchTerm::Subject));
        doc.indexTextWithoutPositions(str, {}, 100);
        doc.setData(str);
    }

    KMime::Content *mainBody = note->mainBodyPart("text/plain");
    if (mainBody) {
        const QString str = mainBody->decodedText();
        doc.indexTextWithoutPositions(str);
        doc.indexText(str, propMapper.prefix(Akonadi::EmailSearchTerm::Body));
    } else {
        processPart(doc, note.get(), nullptr);
    }

    return doc.xapianDocument();
}


void NoteIndexer::processPart(XapianDocument &doc, KMime::Content *content, KMime::Content *mainContent)
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
            for (auto c : contents) {
                processPart(doc, c, mainContent);
            }
        }

        // Only get HTML content, if no plain text content
        if (!mainContent && type->isHTMLText()) {
            QTextDocument textDoc;
            textDoc.setHtml(content->decodedText());
            doc.indexTextWithoutPositions(textDoc.toPlainText());
            doc.indexText(textDoc.toPlainText(), NoteQueryPropertyMapper::instance().prefix(Akonadi::EmailSearchTerm::Body));
        }
    }
}
