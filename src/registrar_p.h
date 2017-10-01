/*
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

#include <functional>

#include <QHash>

namespace Akonadi {
namespace Search {

template<typename Base>
struct Spawner
{
public:
    using Func = std::function<Base*()>;

    Spawner(Func spawner) : mSpawner(spawner) {}
    Base *spawn() const { return mSpawner(); }

private:
    Func mSpawner;
};

template<typename Base, typename Impl>
typename Spawner<Base>::Func spawner()
{
    using SpawnerFunc = typename Spawner<Base>::Func;
    return SpawnerFunc([]() -> Base* { return new Impl{}; });
}

template<typename Base>
class Registrar : public QHash<QString, Spawner<Base>>
{
public:
    using QHash<QString, Spawner<Base>>::QHash;

    template<typename Impl>
    typename std::enable_if<std::is_base_of<Base, Impl>::value, void>::type
    registerForType()
    {
        const auto mts = Impl::mimeTypes();
        for (const auto &mt : mts) {
            this->insertMulti(mt, spawner<Base, Impl>());
        }
    }

    QVector<Base*> spawnInstancesForType(const QString &type) const
    {
        QVector<Base*> rv;
        auto it = this->constFind(type);
        while (it != this->cend() && it.key() == type) {
            rv.push_back(it->spawn());
            ++it;
        }
        return rv;
    }
};


}
}
