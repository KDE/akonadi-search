/*
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "xapianqueryparser.h"

#include "akonadi_search_xapian_debug.h"
#include <QStringList>
#include <QTextBoundaryFinder>

using namespace Akonadi::Search;

XapianQueryParser::XapianQueryParser() = default;

void XapianQueryParser::setDatabase(Xapian::Database *db)
{
    m_db = db;
}

namespace
{
struct Term {
    std::string t;
    uint count;

    // pop_heap pops the largest element, we want the smallest to be popped
    bool operator<(const Term &rhs) const
    {
        return count > rhs.count;
    }
};

Xapian::Query makeQuery(const QString &string, int position, Xapian::Database *db)
{
    if (!db) {
        const QByteArray arr = string.toUtf8();
        const std::string stdString(arr.constData(), arr.size());
        return Xapian::Query(stdString, 1, position);
    }

    // Lets just keep the top x (+1 for push_heap)
    static const int MaxTerms = 100;
    QList<Term> topTerms;
    topTerms.reserve(MaxTerms + 1);

    const std::string stdString(string.toStdString());
    Xapian::TermIterator it = db->allterms_begin(stdString);
    Xapian::TermIterator end = db->allterms_end(stdString);
    for (; it != end; ++it) {
        Term term;
        term.t = *it;
        term.count = db->get_collection_freq(term.t);

        if (topTerms.size() < MaxTerms) {
            topTerms.push_back(term);
            std::push_heap(topTerms.begin(), topTerms.end());
        } else {
            // Remove the term with the min count
            topTerms.push_back(term);
            std::push_heap(topTerms.begin(), topTerms.end());

            std::pop_heap(topTerms.begin(), topTerms.end());
            topTerms.pop_back();
        }
    }

    QList<Xapian::Query> queries;
    queries.reserve(topTerms.size());

    for (const Term &term : std::as_const(topTerms)) {
        queries << Xapian::Query(term.t, 1, position);
    }

    if (queries.isEmpty()) {
        return Xapian::Query(string.toStdString(), 1, position);
    }
    Xapian::Query finalQ(Xapian::Query::OP_SYNONYM, queries.begin(), queries.end());
    return finalQ;
}

bool containsSpace(const QString &string)
{
    for (const QChar &ch : string) {
        if (ch.isSpace()) {
            return true;
        }
    }

    return false;
}
}

Xapian::Query XapianQueryParser::parseQuery(const QString &text, const QString &prefix)
{
    /*
    Xapian::QueryParser parser;
    parser.set_default_op(Xapian::Query::OP_AND);

    if (m_db)
        parser.set_database(*m_db);

    int flags = Xapian::QueryParser::FLAG_PHRASE | Xapian::QueryParser::FLAG_PARTIAL;

    std::string stdString(text.toStdString());
    return parser.parse_query(stdString, flags);
    */

    if (text.isEmpty()) {
        return {};
    }

    QList<Xapian::Query> queries;
    QList<Xapian::Query> phraseQueries;

    int start = 0;
    int end = 0;
    int position = 0;

    bool inDoubleQuotes = false;
    bool inSingleQuotes = false;
    bool inPhrase = false;

    QTextBoundaryFinder bf(QTextBoundaryFinder::Word, text);
    for (; bf.position() != -1; bf.toNextBoundary()) {
        if (bf.boundaryReasons() & QTextBoundaryFinder::StartOfItem) {
            //
            // Check the previous delimiter
            int pos = bf.position();
            if (pos != end) {
                QString delim = text.mid(end, pos - end);
                if (delim.contains(QLatin1Char('"'))) {
                    if (inDoubleQuotes) {
                        queries << Xapian::Query(Xapian::Query::OP_PHRASE, phraseQueries.begin(), phraseQueries.end());
                        phraseQueries.clear();
                        inDoubleQuotes = false;
                    } else {
                        inDoubleQuotes = true;
                    }
                } else if (delim.contains(QLatin1Char('\''))) {
                    if (inSingleQuotes) {
                        queries << Xapian::Query(Xapian::Query::OP_PHRASE, phraseQueries.begin(), phraseQueries.end());
                        phraseQueries.clear();
                        inSingleQuotes = false;
                    } else {
                        inSingleQuotes = true;
                    }
                } else if (!containsSpace(delim)) {
                    if (!inPhrase && !queries.isEmpty()) {
                        phraseQueries << queries.takeLast();
                    }
                    inPhrase = true;
                } else if (inPhrase && !phraseQueries.isEmpty()) {
                    queries << Xapian::Query(Xapian::Query::OP_PHRASE, phraseQueries.begin(), phraseQueries.end());
                    phraseQueries.clear();
                    inPhrase = false;
                }
            }

            start = bf.position();
            continue;
        } else if (bf.boundaryReasons() & QTextBoundaryFinder::EndOfItem) {
            end = bf.position();

            QString str = text.mid(start, end - start);

            // Get the string ready for saving
            str = str.toLower();

            // Remove all accents
            const QString denormalized = str.normalized(QString::NormalizationForm_KD);
            QString cleanString;
            for (const QChar &ch : denormalized) {
                auto cat = ch.category();
                if (cat != QChar::Mark_NonSpacing && cat != QChar::Mark_SpacingCombining && cat != QChar::Mark_Enclosing) {
                    cleanString.append(ch);
                }
            }

            str = cleanString.normalized(QString::NormalizationForm_KC);
            const QList<QStringView> lst = QStringView(str).split(QLatin1Char('_'), Qt::SkipEmptyParts);
            for (const QStringView t : lst) {
                const QString term = prefix + t;

                position++;
                if (inDoubleQuotes || inSingleQuotes || inPhrase) {
                    const QByteArray arr = term.toUtf8();
                    const std::string str(arr.constData(), arr.length());
                    phraseQueries << Xapian::Query(str, 1, position);
                } else {
                    if (m_autoExpand) {
                        queries << makeQuery(term, position, m_db);
                    } else {
                        queries << Xapian::Query(term.toStdString(), 1, position);
                    }
                }
            }
        }
    }

    if (inPhrase) {
        queries << Xapian::Query(Xapian::Query::OP_PHRASE, phraseQueries.begin(), phraseQueries.end());
        phraseQueries.clear();
    }

    if (!phraseQueries.isEmpty()) {
        queries << phraseQueries;
        phraseQueries.clear();
    }

    if (queries.size() == 1) {
        return queries.first();
    }
    return {Xapian::Query::OP_AND, queries.begin(), queries.end()};
}

void XapianQueryParser::setAutoExapand(bool autoexpand)
{
    m_autoExpand = autoexpand;
}

Xapian::Query XapianQueryParser::expandWord(const QString &word, const QString &prefix)
{
    const std::string stdString((prefix + word).toUtf8().constData());
    Xapian::TermIterator it = m_db->allterms_begin(stdString);
    Xapian::TermIterator end = m_db->allterms_end(stdString);

    QList<Xapian::Query> queries;
    for (; it != end; ++it) {
        queries << Xapian::Query(*it);
    }

    if (queries.isEmpty()) {
        return Xapian::Query(stdString);
    }
    Xapian::Query finalQ(Xapian::Query::OP_SYNONYM, queries.begin(), queries.end());
    return finalQ;
}
