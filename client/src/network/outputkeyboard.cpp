/**
 * client/src/network/outputkeyboard.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#include "outputkeyboard.hpp"
#include "connection.hpp"

OutputKeyboard::OutputKeyboard(std::shared_ptr<Connection> connection, Handle handle, const QString& classId_) :
  Object(std::move(connection), handle, classId_),
  m_requestId{Connection::invalidRequestId}
{
  auto request = Message::newRequest(Message::Command::OutputKeyboardGetOutputInfo);
  request->write(m_handle);
  m_connection->send(request,
    [this](const std::shared_ptr<Message> message)
    {
      processMessage(*message);
    });
  m_requestId = request->requestId();
}

OutputKeyboard::~OutputKeyboard()
{
  if(m_requestId != Connection::invalidRequestId)
    m_connection->cancelRequest(m_requestId);
}

TriState OutputKeyboard::getOutputState(uint32_t address) const
{
  if(auto it = m_outputValues.find(address); it != m_outputValues.end())
    return it->second;
  return TriState::Undefined;
}

QString OutputKeyboard::getOutputId(uint32_t address) const
{
  if(auto it = m_outputIds.find(address); it != m_outputIds.end())
    return it->second;
  return {};
}

void OutputKeyboard::outputSetValue(uint32_t address, bool value)
{
  auto event = Message::newEvent(Message::Command::OutputKeyboardSetOutputValue);
  event->write(handle());
  event->write(address);
  event->write(value);
  m_connection->send(event);
}

void OutputKeyboard::processMessage(const Message& message)
{
  switch(message.command())
  {
    case Message::Command::OutputKeyboardGetOutputInfo:
    {
      uint32_t count = message.read<uint32_t>();
      while(count > 0)
      {
        const uint32_t address = message.read<uint32_t>();
        const QString id = QString::fromUtf8(message.read<QByteArray>());
        const TriState value = message.read<TriState>();
        m_outputIds[address] = id;
        m_outputValues[address] = value;
        emit outputIdChanged(address, id);
        emit outputValueChanged(address, value);
        count--;
      }
      return;
    }
    case Message::Command::OutputKeyboardOutputIdChanged:
    {
      const uint32_t address = message.read<uint32_t>();
      const QString id = QString::fromUtf8(message.read<QByteArray>());
      m_outputIds[address] = id;
      emit outputIdChanged(address, id);
      return;
    }
    case Message::Command::OutputKeyboardOutputValueChanged:
    {
      const uint32_t address = message.read<uint32_t>();
      const TriState value = message.read<TriState>();
      m_outputValues[address] = value;
      emit outputValueChanged(address, value);
      return;
    }
    default:
      Object::processMessage(message);
      break;
  }
}
