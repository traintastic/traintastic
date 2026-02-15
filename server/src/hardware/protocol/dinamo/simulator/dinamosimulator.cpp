/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "dinamosimulator.hpp"
#include <cstring>
#include "../dinamomessages.hpp"

namespace Dinamo {

using LockGuard = std::lock_guard<std::recursive_mutex>;

std::tuple<std::span<const uint8_t>, bool, bool> Simulator::process(std::span<const uint8_t> message, bool hold, bool fault)
{
  m_hold = hold;
  m_fault |= fault;

  if(SystemCommand::check(message))
  {
    switch(reinterpret_cast<const SystemCommand*>(message.data())->subCommand)
    {
      case SubCommand::ResetFault:
        m_fault = false;
        break;

      case SubCommand::ProtocolVersion:
        if(message.size() == sizeof(ProtocolVersionRequest))
        {
          // direct response by interface
          return response(ProtocolVersionResponse(3, 2, 0, 0)); // v3.20
        }
        break;

      case SubCommand::SystemVersion:
        if(message.size() == sizeof(SystemVersionRequest))
        {
          // direct response by interface
          return response(SystemVersionResponse(SystemType::RM_C, 1, 4, 0, 4)); // v1.40D
        }
        break;

      default:
        break;
    }
  }
  else if(InputRequestOrResponse::check(message))
  {
    LockGuard lock{m_mutex};
    const auto& msg = *reinterpret_cast<const InputRequestOrResponse*>(message.data());
    auto it = m_inputs.find(msg.address());
    if(it == m_inputs.end())
    {
      it = m_inputs.emplace(msg.address(), false).first;
    }
    // delayed response by subsystem
    queue(InputRequestOrResponse(it->first, it->second));
  }
  return response();
}

void Simulator::inputEvent(uint16_t address, bool value)
{
  LockGuard lock{m_mutex};
  if(auto it = m_inputs.find(address); it != m_inputs.end() && it->second == value)
  {
    return; // no change
  }
  m_inputs[address] = true;
  queue(InputEvent(address, m_inputs[address]));
}

void Simulator::inputEventToggle(uint16_t address)
{
  LockGuard lock{m_mutex};
  if(auto it = m_inputs.find(address); it != m_inputs.end())
  {
    m_inputs[address] = !it->second;
  }
  else
  {
    m_inputs[address] = true;
  }
  queue(InputEvent(address, m_inputs[address]));
}

template<typename T>
void Simulator::queue(const T& message)
{
  LockGuard lock{m_mutex};
  static_assert(std::is_base_of_v<Message, T>);
  const auto* p = reinterpret_cast<const uint8_t*>(&message);
  m_queue.emplace(std::vector<uint8_t>{p, p + sizeof(T)});
}

std::tuple<std::span<const uint8_t>, bool, bool> Simulator::response()
{
  LockGuard lock{m_mutex};
  if(!m_hold && !m_queue.empty())
  {
    const auto& message = m_queue.front();
    const auto size = message.size();
    std::memcpy(m_response.data(), message.data(), size);
    m_queue.pop();
    return {{m_response.data(), size}, false, m_fault};
  }
  return {{}, false, m_fault}; // NULL
}

template<typename T>
std::tuple<std::span<const uint8_t>, bool, bool> Simulator::response(const T& message)
{
  static_assert(std::is_base_of_v<Message, T>);
  std::memcpy(m_response.data(), &message, sizeof(message));
  return {{m_response.data(), sizeof(message)}, false, m_fault};
}

}

