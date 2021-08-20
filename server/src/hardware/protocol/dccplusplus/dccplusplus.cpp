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
#include "../../../log/log.hpp"

namespace DCCPlusPlus {

constexpr std::array<uint8_t, 2> speedStepValues{28, 128};

DCCPlusPlus::DCCPlusPlus(Object& _parent, const std::string& parentPropertyName, std::function<bool(std::string_view)> send) :
  SubObject(_parent, parentPropertyName),
  m_commandStation{dynamic_cast<CommandStation*>(&_parent)},
  m_send{std::move(send)},
  m_debugLogRXTX{false},
  useEx{this, "use_ex", true, PropertyFlags::ReadWrite | PropertyFlags::Store},
  speedSteps{this, "speed_steps", 128, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](uint8_t value)
    {
      this->send(Ex::setSpeedSteps(value));
    }},
  debugLogRXTX{this, "debug_log", m_debugLogRXTX, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool value)
    {
      m_debugLogRXTX = value;
    }}
{
  assert(m_send);

  Attributes::addEnabled(useEx, false); // disable for now, only ex is currently supported
  m_interfaceItems.add(useEx);
  Attributes::addValues(speedSteps, speedStepValues);
  m_interfaceItems.add(speedSteps);
  m_interfaceItems.add(debugLogRXTX);
}

bool DCCPlusPlus::send(std::string_view message)
{
  if(m_debugLogRXTX)
    Log::log(*this, LogMessage::D2001_TX_X, message);
  return m_send(message);
}

void DCCPlusPlus::receive(std::string_view message)
{
  // NOTE: this function is called async!

  if(m_debugLogRXTX)
    EventLoop::call([this, msg=std::string(message)](){ Log::log(*this, LogMessage::D2002_RX_X, msg); });

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

void DCCPlusPlus::checkDecoder(const Decoder& decoder) const
{
  assert(m_commandStation);

  if(decoder.protocol != DecoderProtocol::Auto && decoder.protocol != DecoderProtocol::DCC)
    Log::log(decoder, LogMessage::C2002_DCCPLUSPLUS_ONLY_SUPPORTS_THE_DCC_PROTOCOL);

  if(decoder.protocol == DecoderProtocol::DCC && decoder.address <= 127 && decoder.longAddress)
    Log::log(decoder, LogMessage::C2003_DCCPLUSPLUS_DOESNT_SUPPORT_DCC_LONG_ADDRESSES_BELOW_128);

  if(decoder.speedSteps != Decoder::speedStepsAuto && decoder.speedSteps != speedSteps)
    Log::log(decoder, LogMessage::W2003_COMMAND_STATION_DOESNT_SUPPORT_X_SPEEDSTEPS_USING_X, decoder.speedSteps.value(), speedSteps.value());

  for(const auto& function : *decoder.functions)
    if(function->number > functionNumberMax)
    {
      Log::log(decoder, LogMessage::W2002_COMMAND_STATION_DOESNT_SUPPORT_FUNCTIONS_ABOVE_FX, functionNumberMax);
      break;
    }
}

void DCCPlusPlus::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Throttle | DecoderChangeFlags::Direction))
    send(Ex::setLocoSpeedAndDirection(decoder.address, Decoder::throttleToSpeedStep(decoder.throttle, 126), decoder.emergencyStop, decoder.direction));
  else if(has(changes, DecoderChangeFlags::FunctionValue) && functionNumber <= functionNumberMax)
    send(Ex::setLocoFunction(decoder.address, static_cast<uint8_t>(functionNumber), decoder.getFunctionValue(functionNumber)));
}

void DCCPlusPlus::loaded()
{
  SubObject::loaded();
  m_debugLogRXTX = debugLogRXTX;
}

}