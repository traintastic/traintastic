/**
 * server/src/hardware/protocol/loconet.cpp
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

#include "loconet.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/traintastic.hpp"
#include "../input/loconetinput.hpp"
#include "../../utils/to_hex.hpp"

namespace Protocol {

uint8_t LocoNet::calcChecksum(const Message& msg)
{
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&msg);
  const int size = msg.size() - 1;
  uint8_t checksum = p[0];
  for(int i = 1; i < size; i++)
    checksum ^= p[i];
  return checksum;
}

bool LocoNet::isChecksumValid(const Message& msg)
{
  return calcChecksum(msg) == *(reinterpret_cast<const uint8_t*>(&msg) + msg.size() - 1);
}

LocoNet::LocoNet(Object& parent, const std::string& parentPropertyName, std::function<bool(const Message&)> send) :
  SubObject(parent, parentPropertyName),
  m_send{std::move(send)},
  m_debugLog{true/*false*/},
  debugLog{this, "debug_log", m_debugLog, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  assert(m_send);

  m_interfaceItems.add(debugLog);
}

void LocoNet::receive(const Message& message)
{
  // NOTE: this function is called async!

  /*if(!isChecksumValid(message))
  {
    if(m_debugLog)
    {
      std::string log = "checksum mismatch: ";
      for(int i = 0; i < message.size(); i++)
        log += to_hex(reinterpret_cast<const uint8_t*>(&message)[i]);
      log += " (expected: 0x" + to_hex(calcChecksum(message)) + ")";
      EventLoop::call([this, log](){ Traintastic::instance->console->debug(id(), log); });
    }
    return;
  }*/

  switch(message.opCode)
  {
    case OPC_LOCO_SPD:
    {
      const LocoSpd& locoSpd = static_cast<const LocoSpd&>(message);

      if(m_debugLog)
      {
        const std::string log = "rx OPC_LOCO_SPD:"
          " slot=" + std::to_string(locoSpd.slot) +
          " speed="  + std::to_string(locoSpd.speed);
        EventLoop::call([this, log](){ Traintastic::instance->console->debug(id(), log); });
      }





      break;
    }
    case OPC_INPUT_REP:
    {
      const InputRep& inputRep = static_cast<const InputRep&>(message);

      if(m_debugLog)
      {
        const std::string log = "rx OPC_INPUT_REP:"
          " address=" + std::to_string(inputRep.address()) +
          " input="  + (inputRep.isAuxInput() ? "aux" : "switch") +
          " value=" + (inputRep.value() ? "high" : "low");
        EventLoop::call([this, log](){ Traintastic::instance->console->debug(id(), log); });
      }

      EventLoop::call(
        [this, address=inputRep.address(), value=inputRep.value()]()
        {
          auto it = m_inputs.find(1 + address);
          if(it != m_inputs.end())
            it->second->valueChanged(value);
        });
      break;
    }
    /*
    case OPC_MULTI_SENSE:
    {


      break;
    }
    */
    default:
      if(m_debugLog)
      {
        std::string message = "unknown message: ";
        for(int i = 0; i < message.size(); i++)
          message += to_hex(reinterpret_cast<const uint8_t*>(&message)[i]);
        EventLoop::call([this, message](){ Traintastic::instance->console->debug(id(), message); });
      }
      break;
  }
}

bool LocoNet::isInputAddressAvailable(uint16_t address)
{
  return m_inputs.find(address) == m_inputs.end();
}

bool LocoNet::addInput(const std::shared_ptr<LocoNetInput>& input)
{
  assert(input->loconet.value().get() == this);
  if(isInputAddressAvailable(input->address))
  {
    m_inputs.insert({input->address, input});
    return true;
  }
  else
    return false;
}

void LocoNet::removeInput(const std::shared_ptr<LocoNetInput>& input)
{
  assert(input->loconet.value().get() == this);
  m_inputs.erase(m_inputs.find(input->address));
}

}
