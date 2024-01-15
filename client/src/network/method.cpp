/**
 * client/src/network/method.cpp
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

#include "method.hpp"
#include "object.hpp"
#include "connection.hpp"
#include "error.hpp"

Method::Method(Object& object, const QString& name, ValueType resultType, const QVector<ValueType>& argumentTypes) :
  InterfaceItem(object, name),
  m_resultType{resultType},
  m_argumentTypes{argumentTypes}
{
}

void Method::call()
{
  object().connection()->callMethod(*this);
}

void Method::call(const QString& arg)
{
  object().connection()->callMethod(*this, arg);
}

int Method::call(std::function<void(const ObjectPtr&, std::optional<const Error>)> callback)
{
  return object().connection()->callMethod(*this, std::move(callback));
}

int Method::call(const QString& arg, std::function<void(const ObjectPtr&, std::optional<const Error>)> callback)
{
  return object().connection()->callMethod(*this, arg, std::move(callback));
}
