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

#include "cbussimulator.hpp"
#include <algorithm>
#include "../cbuscanmessageutils.hpp"
#include "../messages/cbusaccessorymessages.hpp"
#include "../messages/cbusaccessoryshortmessages.hpp"

namespace {

static constexpr uint8_t simCanId = 84;
static constexpr uint16_t simNodeNumber = 0x84B7;

}

namespace CBUS
{

Simulator::Simulator()
{
}

bool Simulator::addModule(std::unique_ptr<Module::CANModule> module)
{
  if(!module || std::find_if(m_modules.begin(), m_modules.end(),
    [nodeNumber=module->nodeNumber, canId=module->canId](const auto& m)
    {
      return (m->nodeNumber == nodeNumber) || (m->canId == canId);
    }) != m_modules.end())
  {
    return false;
  }
  m_modules.emplace_back(std::move(module));
  return true;
}

void Simulator::receive(const CAN::Message& canMessage)
{
  for(const auto& module : m_modules)
  {
    module->receive(canMessage);
  }
}

void Simulator::shortEvent(uint16_t eventNumber, SimulateInputAction action)
{
  bool value;
  switch(action)
  {
    case SimulateInputAction::SetTrue:
      value = true;
      break;

    case SimulateInputAction::SetFalse:
      value = false;
      break;

    case SimulateInputAction::Toggle:
      if(auto it = m_shortEvents.find(eventNumber); it != m_shortEvents.end())
      {
        value = !it->second;
      }
      else
      {
        value = true;
      }
      break;

    default: [[unlikely]]
      return;
  }

  if(value)
  {
    send(toCANMessage(AccessoryShortOn(simNodeNumber, eventNumber), simCanId));
  }
  else
  {
    send(toCANMessage(AccessoryShortOff(simNodeNumber, eventNumber), simCanId));
  }
}

void Simulator::longEvent(uint16_t nodeNumber, uint16_t eventNumber, SimulateInputAction action)
{
  uint8_t canId = simCanId;
  if(auto it = std::find_if(m_modules.begin(), m_modules.end(),
    [nodeNumber](const auto& module)
    {
      return module->nodeNumber == nodeNumber;
    }); it != m_modules.end())
  {
    canId = (**it).canId;
  }

  bool value;
  switch(action)
  {
    case SimulateInputAction::SetTrue:
      value = true;
      break;

    case SimulateInputAction::SetFalse:
      value = false;
      break;

    case SimulateInputAction::Toggle:
      if(auto it = m_longEvents.find({nodeNumber, eventNumber}); it != m_longEvents.end())
      {
        value = !it->second;
      }
      else
      {
        value = true;
      }
      break;

    default: [[unlikely]]
      return;
  }

  if(value)
  {
    send(toCANMessage(AccessoryOn(nodeNumber, eventNumber), canId));
  }
  else
  {
    send(toCANMessage(AccessoryOff(nodeNumber, eventNumber), canId));
  }
}

void Simulator::send(const CAN::Message& canMessage)
{
  const auto& message = asMessage(canMessage);
  switch(message.opCode)
  {
    using enum OpCode;

    case ASON:
    {
      const auto& ason = static_cast<const AccessoryShortOn&>(message);
      m_shortEvents[ason.deviceNumber()] = true;
      break;
    }
    case ASOF:
    {
      const auto& asof = static_cast<const AccessoryShortOff&>(message);
      m_shortEvents[asof.deviceNumber()] = false;
      break;
    }
    case ACON:
    {
      const auto& acon = static_cast<const AccessoryOn&>(message);
      m_longEvents[{acon.nodeNumber(), acon.eventNumber()}] = true;
      break;
    }
    case ACOF:
    {
      const auto& acof = static_cast<const AccessoryOff&>(message);
      m_longEvents[{acof.nodeNumber(), acof.eventNumber()}] = false;
      break;
    }
    default:
      break;
  }
  onSend(canMessage);
}

}
