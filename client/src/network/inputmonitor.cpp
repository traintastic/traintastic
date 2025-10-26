/**
 * client/src/network/inputmonitor.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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
#include "event.hpp"

InputMonitor::InputMonitor(const std::shared_ptr<Connection>& connection, Handle handle, const QString& classId_) :
  Object(connection, handle, classId_),
  m_requestId{Connection::invalidRequestId}
{
  auto request = Message::newRequest(Message::Command::InputMonitorGetInputInfo);
  request->write(m_handle);
  m_connection->send(request,
    [this](const std::shared_ptr<Message> message)
    {
      m_requestId = Connection::invalidRequestId;
      assert(message);
      uint32_t count = message->read<uint32_t>();
      while(count > 0)
      {
        const uint32_t address = message->read<uint32_t>();
        auto& inputState = m_inputStates[address];
        inputState.used = message->read<bool>();
        inputState.value = message->read<TriState>();
        emit inputStateChanged(address);
        count--;
      }
    });
  m_requestId = request->requestId();
}

InputMonitor::~InputMonitor()
{
  if(m_requestId != Connection::invalidRequestId)
  {
    m_connection->cancelRequest(m_requestId);
  }
}

const InputMonitor::InputState& InputMonitor::getInputState(uint32_t address) const
{
  static constexpr InputState invalid;

  if(auto it = m_inputStates.find(address); it != m_inputStates.end())
  {
    return it->second;
  }

  return invalid;
}

void InputMonitor::created()
{
  m_simulateInputChange = getMethod("simulate_input_change");
  m_inputUsedChanged = getEvent("input_used_changed");
  m_inputValueChanged = getEvent("input_value_changed");

  connect(m_inputUsedChanged, &Event::fired,
    [this](QVariantList arguments)
    {
      assert(arguments.size() == 2);
      const uint32_t address = arguments[0].toUInt();
      m_inputStates[address].used = arguments[1].toBool();
      emit inputStateChanged(address);
    });

  connect(m_inputValueChanged, &Event::fired,
    [this](QVariantList arguments)
    {
      assert(arguments.size() == 2);
      const uint32_t address = arguments[0].toUInt();
      m_inputStates[address].value = static_cast<TriState>(arguments[1].value<std::underlying_type_t<TriState>>());
      emit inputStateChanged(address);
    });
}

void InputMonitor::simulateInputChange(uint32_t address)
{
  if(m_simulateInputChange)
  {
    ::callMethod(*m_simulateInputChange, nullptr, address);
  }
}
