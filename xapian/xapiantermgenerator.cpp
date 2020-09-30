/*
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "xapiantermgenerator.h"

#include <QTextBoundaryFinder>
#include "akonadi_search_xapian_debug.h"

using namespace Akonadi::Search;

XapianTermGenerator::XapianTermGenerator(Xapian::Document *doc)
    : m_doc(doc)
{
    if (doc) {
        m_termGen.set_document(*doc);
    }
}

void XapianTermGenerator::indexText(const QString &text)
{
    indexText(text, QString());
}

void XapianTermGenerator::setDocument(Xapian::Document *doc)
{
    m_doc = doc;
}

QStringList XapianTermGenerator::termList(const QString &text)
{
    int start = 0;
    int end = 0;

    QStringList list;
    QTextBoundaryFinder bf(QTextBoundaryFinder::Word, text);
    for (; bf.position() != -1; bf.toNextBoundary()) {
        if (bf.boundaryReasons() & QTextBoundaryFinder::StartOfItem) {
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
            cleanString.reserve(denormalized.size());
            for (const QChar &ch : denormalized) {
                auto cat = ch.category();
                if (cat != QChar::Mark_NonSpacing && cat != QChar::Mark_SpacingCombining && cat != QChar::Mark_Enclosing) {
                    cleanString.append(ch);
                }
            }

            str = cleanString.normalized(QString::NormalizationForm_KC);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            list << str.split(QLatin1Char('_'), QString::SkipEmptyParts);
#else
            list << str.split(QLatin1Char('_'), Qt::SkipEmptyParts);
#endif
        }
    }

    return list;
}

void XapianTermGenerator::indexText(const QString &text, const QString &prefix, int wdfInc)
{
    const QByteArray par = prefix.toUtf8();
    //const QByteArray ta = text.toUtf8();
    //m_termGen.index_text(ta.constData(), wdfInc, par.constData());

    const QStringList terms = termList(text);
    for (const QString &term : terms) {
        const QByteArray arr = term.toUtf8();

        const QByteArray finalArr = par + arr;
        const std::string stdString(finalArr.constData(), finalArr.size());
        m_doc->add_posting(stdString, m_position, wdfInc);

        m_position++;
    }
}

int XapianTermGenerator::position() const
{
    return m_position;
}

void XapianTermGenerator::setPosition(int position)
{
    m_position = position;
}
