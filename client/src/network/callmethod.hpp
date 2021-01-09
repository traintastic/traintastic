/**
 * client/src/network/callmethod.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_CALLMETHOD_HPP
#define TRAINTASTIC_CLIENT_NETWORK_CALLMETHOD_HPP

#include "connection.hpp"
#include "method.hpp"
#include <traintastic/network/message.hpp>


// TODO: share with server
#include <type_traits>

template<class T>
struct value_type
{
  static constexpr ValueType value =
    std::is_same_v<T, bool> ? ValueType::Boolean : (
    std::is_enum_v<T> ? ValueType::Enum : (
    std::is_integral_v<T> ? ValueType::Integer : (
    std::is_floating_point_v<T> ? ValueType::Float : (
    std::is_same_v<T, QString> ? ValueType::String : (
    ValueType::Invalid)))));
};

template<typename T>
inline constexpr ValueType value_type_v = value_type<T>::value;


template<class T, class... A>
inline void writeArguments(Message& message, const T& value, A... others)
{
  message.write(value_type_v<T>);
  if constexpr(value_type_v<T> == ValueType::Boolean)
    message.write<bool>(value);
  else if constexpr(value_type_v<T> == ValueType::Integer || value_type_v<T> == ValueType::Enum)
    message.write<int64_t>(static_cast<int64_t>(value));
  else if constexpr(value_type_v<T> == ValueType::Float)
    message.write<double>(value);
  else if constexpr(value_type_v<T> == ValueType::String)
    message.write(value.toUtf8());
  else
    static_assert(sizeof(T) != sizeof(T));

  if constexpr(sizeof...(A) > 0)
    writeArguments(message, others...);
}

template<class R, class... A>
int callMethod(Connection& connection, Method& method, std::function<void(const R&, Message::ErrorCode)> callback, A... args)
{
  auto request = Message::newRequest(Message::Command::ObjectCallMethod);
  request->write(method.object().handle());
  request->write(method.name().toLatin1());
  static_assert(value_type_v<R> != ValueType::Invalid);
  request->write(value_type_v<R>);
  request->write<uint8_t>(sizeof...(A)); // N arguments

  writeArguments(*request, args...);

  connection.send(request,
    [&connection, callback](const std::shared_ptr<Message> message)
    {
      R r;
      if(!message->isError())
      {
        if constexpr(value_type_v<R> == ValueType::Boolean)
          r = message->read<bool>();
        else if constexpr(value_type_v<R> == ValueType::Integer)
          r = message->read<int64_t>();
        else if constexpr(value_type_v<R> == ValueType::Float)
          r = message->read<double>();
        else if constexpr(value_type_v<R> == ValueType::String)
          r = QString::fromUtf8(message->read<QByteArray>());
        else
          static_assert(sizeof(R) != sizeof(R));
      }
      callback(r, message->errorCode());
    });

  return request->requestId();
}

template<class... A>
int callMethod(Method& method, std::function<void(Message::ErrorCode)> callback, A... args)
{
  auto request = Message::newRequest(Message::Command::ObjectCallMethod);
  request->write(method.object().handle());
  request->write(method.name().toLatin1());
  request->write(ValueType::Invalid);
  request->write<uint8_t>(sizeof...(A)); // N arguments

  writeArguments(*request, args...);

  method.object().connection()->send(request,
    [callback=std::move(callback)](const std::shared_ptr<Message> message)
    {
      callback(message->errorCode());
    });

  return request->requestId();
}

template<class... A>
void callMethod(Method& method, std::nullptr_t, A... args)
{
  auto event = Message::newEvent(Message::Command::ObjectCallMethod);
  event->write(method.object().handle());
  event->write(method.name().toLatin1());
  event->write(ValueType::Invalid);
  event->write<uint8_t>(sizeof...(A)); // N arguments

  writeArguments(*event, args...);

  method.object().connection()->send(event);
}

#endif
