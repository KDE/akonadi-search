/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
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

#include "contactcompleter.h"
#include <xapian.h>

#include <KStandardDirs>
#include <KDebug>

using namespace Baloo::PIM;

ContactCompleter::ContactCompleter(const QString& prefix, int limit)
    : m_prefix(prefix.toLower())
    , m_limit(limit)
{

}

QStringList ContactCompleter::complete()
{
    QString dir = KStandardDirs::locateLocal("data", "baloo/emailContacts/");
    Xapian::Database db(dir.toStdString());

    Xapian::QueryParser parser;
    parser.set_database(db);

    std::string prefix = m_prefix.toStdString();
    int flags = Xapian::QueryParser::FLAG_DEFAULT | Xapian::QueryParser::FLAG_PARTIAL;
    Xapian::Query q = parser.parse_query(prefix, flags);

    Xapian::Enquire enq(db);
    enq.set_query(q);

    Xapian::MSet mset = enq.get_mset(0, m_limit);
    Xapian::MSetIterator mit = mset.begin();

    QStringList list;
    for (; mit != mset.end(); mit++) {
        QString entry = QString::fromStdString(mit.get_document().get_data());
        list << entry;
    }

    return list;
}
