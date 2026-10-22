/*
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "xapiandocument.h"

using namespace Akonadi::Search;

XapianDocument::XapianDocument()
    : m_termGen(&m_doc)
{
}

XapianDocument::XapianDocument(const Xapian::Document &doc)
    : m_doc(doc)
    , m_termGen(&m_doc)
{
}

void XapianDocument::addTerm(const QString &term, const QString &prefix)
{
    const QByteArray arr = prefix.toUtf8() + term.toUtf8();

    m_doc.add_term(arr.constData());
}

void XapianDocument::addBoolTerm(int term, const QString &prefix)
{
    addBoolTerm(QString::number(term), prefix);
}

void XapianDocument::addBoolTerm(const QString &term, const QString &prefix)
{
    const QByteArray arr = prefix.toUtf8() + term.toUtf8();

    m_doc.add_boolean_term(arr.constData());
}

void XapianDocument::indexText(const QString &text, const QString &prefix, int wdfInc)
{
    m_termGen.indexText(text, prefix, wdfInc);
}

void XapianDocument::indexText(const QString &text, int wdfInc)
{
    indexText(text, QString(), wdfInc);
}

Xapian::Document XapianDocument::doc() const
{
    return m_doc;
}

void XapianDocument::addValue(int pos, const QString &value)
{
    m_doc.add_value(pos, value.toStdString());
}

QString XapianDocument::fetchTermStartsWith(const QByteArray &term)
{
    try {
        Xapian::TermIterator it = m_doc.termlist_begin();
        it.skip_to(term.constData());

        if (it == m_doc.termlist_end()) {
            return {};
        }
        std::string str = *it;
        return QString::fromUtf8(str.c_str(), str.length());
    } catch (const Xapian::Error &) {
        return {};
    }
}

bool XapianDocument::removeTermStartsWith(const QByteArray &prefix)
{
    bool modified = false;

    Xapian::TermIterator it = m_doc.termlist_begin();
    it.skip_to(prefix.constData());
    while (it != m_doc.termlist_end()) {
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
        m_doc.remove_term(t);
        modified = true;
    }

    return modified;
}
