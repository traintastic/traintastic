/**
 * client/src/network/method.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TRAINTASTIC_CLIENT_NETWORK_METHOD_HPP
#define TRAINTASTIC_CLIENT_NETWORK_METHOD_HPP

#include "interfaceitem.hpp"
#include <optional>
#include <QVector>
#include <traintastic/enum/valuetype.hpp>
#include "objectptr.hpp"

struct Error;

class Method : public InterfaceItem
{
  protected:
    const ValueType m_resultType;
    const QVector<ValueType> m_argumentTypes;

  public:
    Method(Object& object, const QString& name, ValueType resultType, const QVector<ValueType>& argumentTypes);

    ValueType resultType() const { return m_resultType; }
    const QVector<ValueType>& argumentTypes() const { return m_argumentTypes; }

    void call();
    void call(const QString& arg);
    [[nodiscard]] int call(std::function<void(const ObjectPtr&, std::optional<const Error>)> callback);
    [[nodiscard]] int call(const QString& arg, std::function<void(const ObjectPtr&, std::optional<const Error>)> callback);
};

#endif
