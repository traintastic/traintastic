/**
 * client/src/network/property.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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

#include "property.hpp"
#include "connection.hpp"
#include "object.hpp"
#include "error.hpp"

template<class T>
static void setPropertyValue(Property& property, const T& value)
{
  auto event = Message::newEvent(Message::Command::ObjectSetProperty);
  event->write(static_cast<Object*>(property.parent())->handle());
  event->write(property.name().toLatin1());

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

template<class T>
[[nodiscard]] static int setPropertyValue(Property& property, const T& value, std::function<void(std::optional<Error>)> callback)
{
  auto request = Message::newRequest(Message::Command::ObjectSetProperty);
  request->write(static_cast<Object*>(property.parent())->handle());
  request->write(property.name().toLatin1());

  if constexpr(std::is_same_v<T, bool>)
  {
    request->write(ValueType::Boolean);
    request->write(value);
  }
  else if constexpr(std::is_integral_v<T>)
  {
    request->write(ValueType::Integer);
    request->write<int64_t>(value);
  }
  else if constexpr(std::is_floating_point_v<T>)
  {
    request->write(ValueType::Float);
    request->write<double>(value);
  }
  else if constexpr(std::is_same_v<T, QString>)
  {
    request->write(ValueType::String);
    request->write(value.toUtf8());
  }
  else
    static_assert(sizeof(T) != sizeof(T));

  property.object().connection()->send(request,
    [callback](const std::shared_ptr<Message> message)
    {
      if(message->isError())
      {
        callback(*message);
      }
      else // success
      {
        callback({});
      }
    });

  return request->requestId();
}


Property::Property(Object& object, const QString& name, ValueType type, PropertyFlags flags, const QVariant& value) :
  AbstractProperty(object, name, type, flags),
  m_value{value}
{
}

void Property::setValueBool(bool value)
{
  setPropertyValue(*this, value);
}

void Property::setValueInt(int value)
{
  setPropertyValue(*this, value);
}

void Property::setValueInt64(int64_t value)
{
  setPropertyValue(*this, value);
}

void Property::setValueDouble(double value)
{
  setPropertyValue(*this, value);
}

void Property::setValueString(const QString& value)
{
  setPropertyValue(*this, value);
}

int Property::setValueBool(bool value, std::function<void(std::optional<Error> error)> callback)
{
  return setPropertyValue(*this, value, std::move(callback));
}

int Property::setValueInt64(int64_t value, std::function<void(std::optional<Error> error)> callback)
{
  return setPropertyValue(*this, value, std::move(callback));
}

int Property::setValueDouble(double value, std::function<void(std::optional<Error> error)> callback)
{
  return setPropertyValue(*this, value, std::move(callback));
}

int Property::setValueString(const QString& value, std::function<void(std::optional<Error> error)> callback)
{
  return setPropertyValue(*this, value, std::move(callback));
}
