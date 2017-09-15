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
#include "xapiandocument.h"

#include <AkonadiCore/Item>

#include <KMime/Message>

#include <QTextDocument>

using namespace Akonadi::Search;


QStringList NoteIndexer::mimeTypes()
{
    return { QStringLiteral("text/x-vnd.akonadi.note") };
}


Xapian::Document NoteIndexer::index(const Akonadi::Item &item)
{
   KMime::Message::Ptr msg;
    try {
        msg = item.payload<KMime::Message::Ptr>();
    } catch (const Akonadi::PayloadException &) {
        return {};
    }

    XapianDocument doc(process(msg));

    Akonadi::Collection::Id colId = item.parentCollection().id();
    doc.addCollectionTerm(colId);

    return doc.xapianDocument();
}

Xapian::Document NoteIndexer::process(const KMime::Message::Ptr &note)
{
    XapianDocument doc;

    // Process Headers
    // (Give the subject a higher priority)
    KMime::Headers::Subject *subject = note->subject(false);
    if (subject) {
        const QString str = subject->asUnicodeString();
        doc.indexTextWithoutPositions(str, QStringLiteral("SU"));
        doc.indexTextWithoutPositions(str, {}, 100);
        doc.setData(str);
    }

    KMime::Content *mainBody = note->mainBodyPart("text/plain");
    if (mainBody) {
        const QString str = mainBody->decodedText();
        doc.indexTextWithoutPositions(str);
        doc.indexTextWithoutPositions(str, QStringLiteral("BO"), 1);
    } else {
        processPart(doc, note.data(), nullptr);
    }

    return doc.xapianDocument();
}


void NoteIndexer::processPart(XapianDocument &doc, KMime::Content *content, KMime::Content *mainContent)
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
        }
    }
}
