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

#include <QTextBoundaryFinder>

using namespace Akonadi::Search;

namespace Akonadi {
namespace Search {

class XapianTermGeneratorPrivate
{
public:
    Xapian::Document *doc = nullptr;
    Xapian::TermGenerator termGen;

    int position = 1;
};

}
}

XapianTermGenerator::XapianTermGenerator(Xapian::Document *doc)
    : d(new XapianTermGeneratorPrivate)
{
    // Turn off stemming to get predictable results for field searches and
    // to avoid stemming non-English data
    d->termGen.set_stemming_strategy(Xapian::TermGenerator::STEM_NONE);
    d->termGen.set_stopper_strategy(Xapian::TermGenerator::STOP_NONE);
    d->doc = doc;
    if (doc) {
        d->termGen.set_document(*doc);
    }
}

XapianTermGenerator::~XapianTermGenerator()
{
    delete d;
}

void XapianTermGenerator::indexText(const QString &text)
{
    indexText(text, {});
}

void XapianTermGenerator::setDocument(Xapian::Document *doc)
{
    d->doc = doc;
}

QStringList XapianTermGenerator::termList(const QString &text)
{
    int start = 0;
    int end = 0;

    QStringList list = { QStringLiteral("^") };
    QTextBoundaryFinder bf(QTextBoundaryFinder::Word, text);
    for (; bf.position() != -1; bf.toNextBoundary()) {
        if (bf.boundaryReasons() & QTextBoundaryFinder::StartOfItem) {
            start = bf.position();
            continue;
        } else if (bf.boundaryReasons() & QTextBoundaryFinder::EndOfItem) {
            end = bf.position();

            QString str = text.mid(start, end - start);

            str = str.normalized(QString::NormalizationForm_KC);

            // Get the string ready for saving
            str = str.toLower();
            list << str.split(QLatin1Char('_'), QString::SkipEmptyParts);
        }
    }
    list << QStringLiteral("$");
    return list;
}

void XapianTermGenerator::indexText(const QString &text, const std::string &prefix, int wdfInc)
{
    //const QByteArray ta = text.toUtf8();
    //m_termGen.index_text(ta.constData(), wdfInc, par.constData());

    QStringList terms = termList(text);
    for (const QString &term : terms) {
        QByteArray arr = term.toUtf8();

        std::string finalArr = prefix + arr.constData();
        d->doc->add_posting(finalArr, d->position, wdfInc);

        d->position++;
    }
}

void XapianTermGenerator::indexTextWithoutPositions(const QString &text, const std::string &prefix, int wdfInc)
{
    auto normalized = text.normalized(QString::NormalizationForm_KC);
    d->termGen.index_text_without_positions(normalized.toStdString(), wdfInc, prefix);
}


int XapianTermGenerator::position() const
{
    return d->position;
}

void XapianTermGenerator::setPosition(int position)
{
    d->position = position;
}

