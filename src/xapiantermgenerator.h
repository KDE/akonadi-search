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

#ifndef AKONADI_SEARCH_XAPIANTERMGENERATOR_H_
#define AKONADI_SEARCH_XAPIANTERMGENERATOR_H_

#include "akonadisearch_export.h"

#include <QString>
#include <QStringList>

namespace Xapian {
class Document;
}

namespace Akonadi
{
namespace Search
{

class XapianTermGeneratorPrivate;
class AKONADISEARCH_EXPORT XapianTermGenerator
{
public:
    explicit XapianTermGenerator(Xapian::Document *doc);
    ~XapianTermGenerator();

    void indexText(const QString &text);
    void indexText(const QString &text, const QString &prefix, int wdfInc = 1);
    void indexTextWithoutPositions(const QString &text, const QString &prefix, int wdfInc = 1);

    void setPosition(int position);
    int position() const;

    void setDocument(Xapian::Document *doc);

    static QStringList termList(const QString &text);

private:
    XapianTermGeneratorPrivate * const d;
};
}
}

#endif // AKONADI_SEARCH_XAPIANTERMGENERATOR_H_
