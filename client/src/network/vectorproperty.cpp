/**
 * client/src/network/vectorproperty.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "vectorproperty.hpp"
#include "connection.hpp"
#include "object.hpp"

template<class T>
static void setVectorPropertyValue(VectorProperty& property, int index, const T& value)
{
  auto event = Message::newEvent(Message::Command::ObjectSetVectorProperty);
  event->write(static_cast<Object*>(property.parent())->handle());
  event->write(property.name().toLatin1());
  event->write<uint32_t>(index);

  if constexpr(std::is_same_v<T, bool>)
  {
    event->write(ValueType::Boolean);
    event->write(value);
  }
  else if constexpr(std::is_integral_v<T>)
  {
    event->write(ValueType::Integer);
    event->write<int64_t>(value);
  }
  else if constexpr(std::is_floating_point_v<T>)
  {
    event->write(ValueType::Float);
    event->write<double>(value);
  }
  else if constexpr(std::is_same_v<T, QString>)
  {
    event->write(ValueType::String);
    event->write(value.toUtf8());
  }
  else
    static_assert(sizeof(T) != sizeof(T));

  property.object().connection()->send(event);
}

void VectorProperty::setBool(int index, bool value)
{
  setVectorPropertyValue(*this, index, value);
}

void VectorProperty::setInt(int index, int value)
{
  setVectorPropertyValue(*this, index, value);
}

void VectorProperty::setInt64(int index, qint64 value)
{
  setVectorPropertyValue(*this, index, value);
}

void VectorProperty::setDouble(int index, double value)
{
  setVectorPropertyValue(*this, index, value);
}

void VectorProperty::setString(int index, const QString& value)
{
  setVectorPropertyValue(*this, index, value);
}
