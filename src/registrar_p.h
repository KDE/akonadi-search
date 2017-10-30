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
struct Instantiator
{
public:
    using Func = std::function<Base*()>;

    Instantiator(Func inst) : mInstantiator(inst) {}
    Base *instantiate() const { return mInstantiator(); }

private:
    Func mInstantiator;
};

template<typename Base, typename Impl>
typename Instantiator<Base>::Func instantiator()
{
    using InstantiatorFunc = typename Instantiator<Base>::Func;
    return InstantiatorFunc([]() -> Base* { return new Impl{}; });
}

template<typename Base>
class Registrar : public QHash<QString, Instantiator<Base>>
{
public:
    using QHash<QString, Instantiator<Base>>::QHash;

    template<typename Impl>
    typename std::enable_if<std::is_base_of<Base, Impl>::value, void>::type
    registerForType()
    {
        const auto mts = Impl::mimeTypes();
        for (const auto &mt : mts) {
            this->insertMulti(mt, instantiator<Base, Impl>());
        }
    }

    QVector<Base*> instantiate(const QString &type) const
    {
        QVector<Base*> rv;
        auto it = this->constFind(type);
        while (it != this->cend() && it.key() == type) {
            rv.push_back(it->instantiate());
            ++it;
        }
        return rv;
    }
};


}
}
