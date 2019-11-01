/**
 * shared/src/message.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#include "message.hpp"

std::atomic<uint16_t> Message::s_requestId(0);

std::unique_ptr<Message> Message::newRequest(Command command, size_t capacity)
{
  return std::make_unique<Message>(command, Type::Request, ++s_requestId, capacity);
}

std::unique_ptr<Message> Message::newResponse(Command command, uint16_t requestId, size_t capacity)
{
  return std::make_unique<Message>(command, Type::Response, requestId, capacity);
}

std::unique_ptr<Message> Message::newErrorResponse(Command command, uint16_t requestId, ErrorCode errorCode)
{
  std::unique_ptr<Message> message = std::make_unique<Message>(command, Type::Response, requestId, errorStr.length());
  message->header()->flags.errorCode = static_cast<uint8_t>(errorCode);
  message->add(errorStr);
  return std::move(message);
}

std::unique_ptr<Message> Message::newEvent(Command command, size_t capacity)
{
  return std::make_unique<Message>(command, Type::Event, 0, capacity);
}

Message::Message(const Header& _header) :
  m_data(sizeof(Header) + _header.dataSize, 0),
  m_readPosition{0}
{
  *header() = _header;
}

Message::Message(Command command, Type type, uint16_t requestId, size_t capacity) :
  m_data(sizeof(Header), 0),
  m_readPosition{0}
{
  header()->command = command;
  header()->flags.type = static_cast<uint8_t>(type);
  header()->requestId = requestId;
  header()->dataSize = 0;

  if(capacity > 0)
    m_data.reserve(sizeof(Header) + capacity);
}

Message::~Message()
{
}



std::string Message::read() const
{
  uint32_t length = read<uint32_t>();
  const uint8_t* start = m_data.data() + m_readPosition;
  std::string value(start, start + length);
  m_readPosition += length;
  return value;
}



void Message::add(const std::string& value)
{
  add<uint32_t>(value.length());
  const size_t oldSize = m_data.size();
  m_data.resize(oldSize + value.length());
  memcpy(m_data.data() + oldSize, value.data(), value.length());
  header()->dataSize += value.length();
}

#ifdef QT_CORE_LIB
void Message::add(const QString& value)
{
  const QByteArray data = value.toUtf8();
  add<uint32_t>(data.length());
  const size_t oldSize = m_data.size();
  m_data.resize(oldSize + data.length());
  memcpy(m_data.data() + oldSize, data.data(), data.length());
  header()->dataSize += data.length();
}
#endif

const Message::Header* Message::header() const
{
  return reinterpret_cast<const Header*>(m_data.data());
}

Message::Header* Message::header()
{
  return reinterpret_cast<Header*>(m_data.data());
}
