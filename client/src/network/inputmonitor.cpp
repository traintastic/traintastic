/**
 * client/src/network/inputmonitor.cpp
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

#include "inputmonitor.hpp"
#include "connection.hpp"
#include "callmethod.hpp"

InputMonitor::InputMonitor(const std::shared_ptr<Connection>& connection, Handle handle, const QString& classId_) :
  Object(connection, handle, classId_),
  m_requestId{Connection::invalidRequestId}
{
  auto request = Message::newRequest(Message::Command::InputMonitorGetInputInfo);
  request->write(m_handle);
  m_connection->send(request,
    [this](const std::shared_ptr<Message> message)
    {
      processMessage(*message);
    });
  m_requestId = request->requestId();
}

InputMonitor::~InputMonitor()
{
  if(m_requestId != Connection::invalidRequestId)
    m_connection->cancelRequest(m_requestId);
}

TriState InputMonitor::getInputState(uint32_t address) const
{
  if(auto it = m_inputValues.find(address); it != m_inputValues.end())
    return it->second;
  return TriState::Undefined;
}

QString InputMonitor::getInputId(uint32_t address) const
{
  if(auto it = m_inputIds.find(address); it != m_inputIds.end())
    return it->second;
  return {};
}

void InputMonitor::simulateInputChange(uint32_t address)
{
  if(auto* simulateInputChange = getMethod("simulate_input_change"))
    ::callMethod(*simulateInputChange, nullptr, address);
}

void InputMonitor::processMessage(const Message& message)
{
  switch(message.command())
  {
    case Message::Command::InputMonitorGetInputInfo:
    {
      uint32_t count = message.read<uint32_t>();
      while(count > 0)
      {
        const uint32_t address = message.read<uint32_t>();
        const QString id = QString::fromUtf8(message.read<QByteArray>());
        const TriState value = message.read<TriState>();
        m_inputIds[address] = id;
        m_inputValues[address] = value;
        emit inputIdChanged(address, id);
        emit inputValueChanged(address, value);
        count--;
      }
      return;
    }
    case Message::Command::InputMonitorInputIdChanged:
    {
      const uint32_t address = message.read<uint32_t>();
      const QString id = QString::fromUtf8(message.read<QByteArray>());
      m_inputIds[address] = id;
      emit inputIdChanged(address, id);
      return;
    }
    case Message::Command::InputMonitorInputValueChanged:
    {
      const uint32_t address = message.read<uint32_t>();
      const TriState value = message.read<TriState>();
      m_inputValues[address] = value;
      emit inputValueChanged(address, value);
      return;
    }
    default:
      Object::processMessage(message);
      break;
  }
}
