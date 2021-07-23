/**
 * server/src/hardware/protocol/dccplusplus/dccplusplus.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "dccplusplus.hpp"
#include "messages.hpp"
#include "../../../core/eventloop.hpp"
#include "../../commandstation/commandstation.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../../core/attributes.hpp"

namespace DCCPlusPlus {

DCCPlusPlus::DCCPlusPlus(Object& _parent, const std::string& parentPropertyName, std::function<bool(std::string_view)> send) :
  SubObject(_parent, parentPropertyName),
  m_commandStation{dynamic_cast<CommandStation*>(&_parent)},
  m_send{std::move(send)},
  m_debugLogRXTX{false},
  useEx{this, "use_ex", true, PropertyFlags::ReadWrite | PropertyFlags::Store},
  debugLogRXTX{this, "debug_log", m_debugLogRXTX, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool value)
    {
      m_debugLogRXTX = value;
    }}
{
  assert(m_send);

  //! \todo 28/128 speed steps

  Attributes::addEnabled(useEx, false); // disable for now, only ex is currently supported
  m_interfaceItems.add(useEx);
  m_interfaceItems.add(debugLogRXTX);
}

bool DCCPlusPlus::send(std::string_view message)
{
  if(m_debugLogRXTX)
    logDebug("tx: " + std::string(message));
  return m_send(message);
}

void DCCPlusPlus::receive(std::string_view message)
{
  // NOTE: this function is called async!

  if(m_debugLogRXTX)
    EventLoop::call([this, log="rx: " + std::string(message)](){ logDebug(log); });

  if(message.size() > 1 && message[0] == '<')
  {
    switch(message[1])
    {
      case 'p': // Power on/off response
        if(m_commandStation)
        {
          if(message[2] == '0')
            m_commandStation->powerOn.setValueInternal(false);
          else if(message[2] == '1')
            m_commandStation->powerOn.setValueInternal(true);
        }
        break;
    }
  }
}

void DCCPlusPlus::emergencyStopChanged(bool value)
{
  if(value)
    send(Ex::emergencyStop());
}

void DCCPlusPlus::powerOnChanged(bool value)
{
  if(value)
    send(Ex::powerOn());
  else
    send(Ex::powerOff());
}

void DCCPlusPlus::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  //! \todo check protocol and long address

  if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Throttle | DecoderChangeFlags::Direction))
    send(Ex::setLocoSpeedAndDirection(decoder.address, Decoder::throttleToSpeedStep(decoder.throttle, 126), decoder.emergencyStop, decoder.direction));

  if(has(changes, DecoderChangeFlags::FunctionValue))
  {
    if(functionNumber <= 68)
      send(Ex::setLocoFunction(decoder.address, static_cast<uint8_t>(functionNumber), decoder.getFunctionValue(functionNumber)));
    else
      logWarning("Function F" + std::to_string(functionNumber) + " not supported");
  }
}

void DCCPlusPlus::loaded()
{
  SubObject::loaded();
  m_debugLogRXTX = debugLogRXTX;
}

}
