/**
 * server/src/hardware/protocol/dccex/iohandler/simulationiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2024-2025 Reinder Feenstra
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

#include "simulationiohandler.hpp"
#include "../kernel.hpp"
#include "../messages.hpp"
#include "../../dcc/dcc.hpp"
#include "../../../../utils/endswith.hpp"
#include "../../../../utils/fromchars.hpp"
#include "../../../../utils/inrange.hpp"

namespace DCCEX {

SimulationIOHandler::SimulationIOHandler(Kernel& kernel)
  : IOHandler(kernel)
{
}

void SimulationIOHandler::setSimulator(std::string hostname, uint16_t port)
{
  assert(!m_simulator);
  m_simulator = std::make_unique<SimulatorIOHandler>(m_kernel.ioContext(), std::move(hostname), port);
}

void SimulationIOHandler::start()
{
  if(m_simulator)
  {
    m_simulator->onPower =
      [this](bool powerOn)
      {
        reply(powerOn ? Messages::powerOnResponse() : Messages::powerOffResponse());
      };
    m_simulator->onLocomotiveSpeedDirection =
      [this](DecoderProtocol protocol, uint16_t address, uint8_t speed, Direction direction, bool emergencyStop)
      {
        if((protocol == DecoderProtocol::None || protocol == DecoderProtocol::DCCShort || protocol == DecoderProtocol::DCCLong) &&
            inRange(address, DCC::addressMin, DCC::addressLongMax))
        {
          reply(Messages::locoBroadcast(address, -1, speed, emergencyStop, direction, 0));
        }
      };
    // FIXME: m_simulator->onAccessorySetState
    // DCC-EX does not confirm/broadcast accessory commands (yet).
    m_simulator->onSensorChanged =
      [this](uint16_t /*channel*/, uint16_t address, bool value)
      {
        if(inRange(address, Messages::sensorIdMin, Messages::sensorIdMax))
        {
          reply(Messages::sensorTransition(address, value));
        }
      };
    m_simulator->start();
  }
  m_kernel.started();
}

bool SimulationIOHandler::send(std::string_view message)
{
  if(message.size() < 4 || message[0] != '<' || !endsWith(message, ">\n"))
    return false;

  switch(message[1])
  {
    case '0': // power off
      if(message == Messages::powerOff())
      {
        if(m_simulator)
        {
          m_simulator->sendPower(false);
        }
        reply(Messages::powerOffResponse());
      }
      else if(message == Messages::powerOff(Messages::Track::Main))
        reply(Messages::powerOffResponse(Messages::Track::Main));
      else if(message == Messages::powerOff(Messages::Track::Programming))
        reply(Messages::powerOffResponse(Messages::Track::Programming));
      break;

    case '1': // power on
      if(message == Messages::powerOn())
      {
        if(m_simulator)
        {
          m_simulator->sendPower(true);
        }
        reply(Messages::powerOnResponse());
      }
      else if(message == Messages::powerOn(Messages::Track::Main))
        reply(Messages::powerOnResponse(Messages::Track::Main));
      else if(message == Messages::powerOn(Messages::Track::Programming))
        reply(Messages::powerOnResponse(Messages::Track::Programming));
      else if(message == Messages::powerOnJoin())
        reply(Messages::powerOnJoinResponse());
      break;

    case 'a': // Accessory
      if(m_simulator)
      {
        uint16_t address;
        auto r = fromChars(message.substr(3), address);
        if(r.ec != std::errc())
        {
          break;
        }
        uint8_t value;
        r = fromChars(message.substr(r.ptr - message.data() + 1), value);
        if(r.ec != std::errc())
        {
          break;
        }
        m_simulator->sendAccessorySetState(0, address, 1 + value);
      }
      break;

    case 't': // LocoSpeedAndDirection
      if(m_simulator)
      {
        uint8_t one;
        auto r = fromChars(message.substr(3), one);
        if(r.ec != std::errc() || one != 1)
        {
          break;
        }
        uint16_t address;
        r = fromChars(message.substr(r.ptr - message.data() + 1), address);
        if(r.ec != std::errc())
        {
          break;
        }
        int8_t speed;
        r = fromChars(message.substr(r.ptr - message.data() + 1), speed);
        if(r.ec != std::errc())
        {
          break;
        }
        uint8_t direction;
        r = fromChars(message.substr(r.ptr - message.data() + 1), direction);
        if(r.ec != std::errc() || (direction != 0 && direction != 1))
        {
          break;
        }
        m_simulator->sendLocomotiveSpeedDirection(
          DCC::getProtocol(address),
          address,
          static_cast<uint8_t>(speed > 0 ? speed : 0),
          direction == 1 ? Direction::Forward : Direction::Reverse,
          speed == -1);
      }
      break;

    case 'T': // Turnout
    {
      uint16_t id;
      if(auto r = fromChars(message.substr(3), id); r.ec == std::errc())
      {
        const char state = *(r.ptr + 1);

        if(state == '0' || state == 'C')
          reply(Messages::setTurnoutResponse(id, false));
        else if(state == '1' || state == 'T')
          reply(Messages::setTurnoutResponse(id, true));
      }
      break;
    }
    case 'Z': // Output
    {
      uint16_t id;
      if(auto r = fromChars(message.substr(3), id); r.ec == std::errc())
      {
        const char state = *(r.ptr + 1);

        if(state == '0')
          reply(Messages::setOutputResponse(id, false));
        else if(state == '1')
          reply(Messages::setOutputResponse(id, true));
      }
      break;
    }
  }

  return true;
}

void SimulationIOHandler::reply(std::string_view message)
{
  // post the reply, so it has some delay
  //! \todo better delay simulation? at least DCC-EX message transfer time?
  m_kernel.ioContext().post(
    [this, data=std::string(message)]()
    {
      m_kernel.receive(data);
    });
}

}
