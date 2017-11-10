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

#include "contactindexer.h"
#include "contactquerypropertymapper.h"
#include "xapiandocument.h"
#include "akonadisearch_debug.h"

#include <AkonadiCore/Item>
#include <AkonadiCore/SearchQuery>

#include <KContacts/Addressee>

using namespace Akonadi::Search;

QStringList ContactIndexer::mimeTypes()
{
    return { KContacts::Addressee::mimeType() };
}

Xapian::Document ContactIndexer::doIndex(const Akonadi::Item &item)
{
    KContacts::Addressee addressee;
    try {
        addressee = item.payload<KContacts::Addressee>();
    } catch (const Akonadi::PayloadException &e) {
        qCWarning(AKONADISEARCH_LOG) << "Item" << item.id() << "does not contain the expected payload:" << e.what();
        return {};
    }

    XapianDocument doc;

    QString name;
    if (!addressee.formattedName().isEmpty()) {
        name = addressee.formattedName();
    } else if (!addressee.assembledName().isEmpty()) {
        name = addressee.assembledName();
    } else {
        name = addressee.name();
    }

    const auto &propMapper = ContactQueryPropertyMapper::instance();
    doc.indexText(addressee.uid(), propMapper.prefix(Akonadi::ContactSearchTerm::Uid));
    doc.indexText(name, propMapper.prefix(Akonadi::ContactSearchTerm::Name));
    doc.indexText(addressee.nickName(), propMapper.prefix(Akonadi::ContactSearchTerm::Nickname));

    const QStringList lstEmails = addressee.emails();
    for (const QString &email : lstEmails) {
        doc.addTerm(email);
        doc.indexText(email);
    }

    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Akonadi::Search::ContactIndexer::index",
               "Item does not have a valid parent collection");

    const Akonadi::Collection::Id colId = item.parentCollection().id();
    doc.addCollectionTerm(colId);

    if (addressee.birthday().isValid()) {
        doc.addValue(propMapper.valueProperty(Akonadi::ContactSearchTerm::Birthday),
                     addressee.birthday().date().toJulianDay());
    }

    const auto annStr = addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Anniversary"));
    if (!annStr.isEmpty()) {
        doc.addValue(propMapper.valueProperty(Akonadi::ContactSearchTerm::Anniversary),
                     QDate::fromString(annStr, Qt::ISODate).toJulianDay());
    }

    return doc.xapianDocument();
}

