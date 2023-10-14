/**
 * client/src/network/callmethod.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2023 Reinder Feenstra
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
#include "object.hpp"
#include "error.hpp"
#include <traintastic/network/message.hpp>
#include <traintastic/utils/valuetypetraits.hpp>

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
  else if constexpr(value_type_v<T> == ValueType::Object)
    message.write(value->handle());
  else
    static_assert(sizeof(T) != sizeof(T));

  if constexpr(sizeof...(A) > 0)
    writeArguments(message, others...);
}

template<typename R>
inline R getResult(Connection& connection, const Message& message)
{
  if constexpr(value_type_v<R> == ValueType::Boolean)
    return message.read<bool>();
  else if constexpr(value_type_v<R> == ValueType::Integer)
    return message.read<int64_t>();
  else if constexpr(value_type_v<R> == ValueType::Float)
    return message.read<double>();
  else if constexpr(value_type_v<R> == ValueType::String)
    return QString::fromUtf8(message.read<QByteArray>());
  else if constexpr(value_type_v<R> == ValueType::Object)
    return connection.readObject(message);
  else
    static_assert(sizeof(R) != sizeof(R));
}

template<class R, class... A>
int callMethod(Connection& connection, Method& method, std::function<void(const R&, std::optional<const Error>)> callback, A... args)
{
  auto request = Message::newRequest(Message::Command::ObjectCallMethod);
  request->write(method.object().handle());
  request->write(method.name().toLatin1());
  static_assert(value_type_v<R> != ValueType::Invalid);
  request->write(value_type_v<R>);
  request->write<uint8_t>(sizeof...(A)); // N arguments

  if constexpr(sizeof...(A) > 0)
    writeArguments(*request, args...);

  connection.send(request,
    [&connection, callback](const std::shared_ptr<Message> message)
    {
      if(!message->isError())
      {
        callback(getResult<R>(connection, *message), {});
      }
      else
      {
        callback(R(), *message);
      }
    });

  return request->requestId();
}

template<class R, class... A>
int callMethodR(Method& method, std::function<void(const R&, std::optional<const Error>)> callback, A... args)
{
  auto request = Message::newRequest(Message::Command::ObjectCallMethod);
  request->write(method.object().handle());
  request->write(method.name().toLatin1());
  static_assert(value_type_v<R> != ValueType::Invalid);
  request->write(value_type_v<R>);
  request->write<uint8_t>(sizeof...(A)); // N arguments

  if constexpr(sizeof...(A) > 0)
    writeArguments(*request, args...);

  auto c = method.object().connection();
  c->send(request,
    [c, callback=std::move(callback)](const std::shared_ptr<Message> message)
    {
      if(!message->isError())
      {
        callback(getResult<R>(*c, *message), {});
      }
      else
      {
        callback(R(), *message);
      }
    });

  return request->requestId();
}

template<class... A>
int callMethod(Method& method, std::function<void(std::optional<const Error>)> callback, A... args)
{
  auto request = Message::newRequest(Message::Command::ObjectCallMethod);
  request->write(method.object().handle());
  request->write(method.name().toLatin1());
  request->write(ValueType::Invalid);
  request->write<uint8_t>(sizeof...(A)); // N arguments

  if constexpr(sizeof...(A) > 0)
    writeArguments(*request, args...);

  method.object().connection()->send(request,
    [callback=std::move(callback)](const std::shared_ptr<Message> message)
    {
      callback(message->isError() ? std::optional<const Error>(*message) : std::nullopt);
    });

  return request->requestId();
}

template<class... A>
void callMethod(Method& method, std::nullptr_t = nullptr, A... args)
{
  auto event = Message::newEvent(Message::Command::ObjectCallMethod);
  event->write(method.object().handle());
  event->write(method.name().toLatin1());
  event->write(ValueType::Invalid);
  event->write<uint8_t>(sizeof...(A)); // N arguments

  if constexpr(sizeof...(A) > 0)
    writeArguments(*event, args...);

  method.object().connection()->send(event);
}

#endif
