/*
 * Copyright (C) 2018  Daniel Vrátil <dvratil@kde.org>
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

#include "utils.h"

#include <xapian.h>

#include <QDataStream>

QDataStream &operator<<(QDataStream &stream, const Xapian::Document &document)
{
    const std::string raw = document.serialise();
    const auto size = raw.size();
    stream.writeRawData(reinterpret_cast<const char *>(&size), sizeof(std::string::size_type));
    stream.writeRawData(raw.c_str(), raw.size());
    return stream;
}


QDataStream &operator>>(QDataStream &stream, Xapian::Document &document)
{
    std::string::size_type size;
    stream.readRawData(reinterpret_cast<char*>(&size), sizeof(std::string::size_type));
    std::string string(size, '\0');
    stream.readRawData(&string.front(), size);
    document = Xapian::Document::unserialise(string);
    return stream;
}
