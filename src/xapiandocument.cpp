/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <xapian.h>

#include "xapiantermgenerator.h"
#include "xapiandocument.h"

using namespace Akonadi::Search;

namespace Akonadi {
namespace Search {

class XapianDocumentPrivate
{
public:
    XapianDocumentPrivate()
        : termGen(&doc)
    {}

    XapianDocumentPrivate(const Xapian::Document &doc)
        : doc(doc)
        , termGen(&this->doc)
    {}

    Xapian::Document doc;
    XapianTermGenerator termGen;
};

}
}

XapianDocument::XapianDocument()
    : d(new XapianDocumentPrivate)
{
}

XapianDocument::XapianDocument(const Xapian::Document &doc)
    : d(new XapianDocumentPrivate(doc))
{
}

XapianDocument::~XapianDocument()
{
    delete d;
}

void XapianDocument::addCollectionTerm(qint64 colId)
{
    d->doc.add_boolean_term(collectionId(colId).constData());
}

void XapianDocument::addTerm(const QString &term, const QString &prefix)
{
    const QByteArray arr = prefix.toUtf8() + term.toUtf8();

    d->doc.add_term(arr.constData());
}

void XapianDocument::addBoolTerm(int term, const QString &prefix)
{
    addBoolTerm(QString::number(term), prefix);
}

void XapianDocument::addBoolTerm(const QString &term, const QString &prefix)
{
    const QByteArray arr = prefix.toUtf8() + term.toUtf8();

    d->doc.add_boolean_term(arr.constData());
}

void XapianDocument::indexText(const QString &text, const QString &prefix, int wdfInc)
{
    d->termGen.indexText(text, prefix, wdfInc);
}

void XapianDocument::indexText(const QString &text, int wdfInc)
{
    indexText(text, QString(), wdfInc);
}

void XapianDocument::indexTextWithoutPositions(const QString &text, const QString &prefix, int wfdInc)
{
    d->termGen.indexTextWithoutPositions(text, prefix, wfdInc);
}

void XapianDocument::setData(const QString &text)
{
    d->doc.set_data(text.toStdString());
}

Xapian::Document XapianDocument::xapianDocument() const
{
    return d->doc;
}

void XapianDocument::addValue(int pos, const QString &value)
{
    d->doc.add_value(pos, value.toUtf8().constData());
}

QString XapianDocument::fetchTermStartsWith(const QByteArray &term)
{
    try {
        Xapian::TermIterator it = d->doc.termlist_begin();
        it.skip_to(term.constData());

        if (it == d->doc.termlist_end()) {
            return QString();
        }
        std::string str = *it;
        return QString::fromUtf8(str.c_str(), str.length());
    } catch (const Xapian::Error &) {
        return QString();
    }
}

bool XapianDocument::removeTermStartsWith(const QByteArray &prefix)
{
    bool modified = false;

    Xapian::TermIterator it = d->doc.termlist_begin();
    it.skip_to(prefix.constData());
    while (it != d->doc.termlist_end()) {
        const std::string t = *it;
        const QByteArray term = QByteArray::fromRawData(t.c_str(), t.size());
        if (!term.startsWith(prefix)) {
            break;
        }

        // The term should not just be the prefix
        if (term.size() <= prefix.size()) {
            break;
        }

        // The term should not contain any more upper case letters
        if (isupper(term.at(prefix.size()))) {
            ++it;
            continue;
        }

        ++it;
        d->doc.remove_term(t);
        modified = true;
    }

    return modified;
}

bool XapianDocument::removeTerm(const QByteArray &term)
{
    d->doc.remove_term(term.constData());
    return true;
}


QByteArray XapianDocument::collectionId(qint64 id)
{
    return 'C' + QByteArray::number(id);
}
