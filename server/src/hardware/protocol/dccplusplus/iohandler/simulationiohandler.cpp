/**
 * server/src/hardware/protocol/dccplusplus/iohandler/simulationiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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
#include "../../../../utils/endswith.hpp"
#include "../../../../utils/fromchars.hpp"

namespace DCCPlusPlus {

SimulationIOHandler::SimulationIOHandler(Kernel& kernel)
  : IOHandler(kernel)
{
}

bool SimulationIOHandler::send(std::string_view message)
{
  if(message.size() < 4 || message[0] != '<' || !endsWith(message, ">\n"))
    return false;

  switch(message[1])
  {
    case '0': // power off
      if(message == Ex::powerOff())
        reply(Ex::powerOffResponse());
      else if(message == Ex::powerOff(Ex::Track::Main))
        reply(Ex::powerOffResponse(Ex::Track::Main));
      else if(message == Ex::powerOff(Ex::Track::Programming))
        reply(Ex::powerOffResponse(Ex::Track::Programming));
      break;

    case '1': // power on
      if(message == Ex::powerOn())
        reply(Ex::powerOnResponse());
      else if(message == Ex::powerOn(Ex::Track::Main))
        reply(Ex::powerOnResponse(Ex::Track::Main));
      else if(message == Ex::powerOn(Ex::Track::Programming))
        reply(Ex::powerOnResponse(Ex::Track::Programming));
      else if(message == Ex::powerOnJoin())
        reply(Ex::powerOnJoinResponse());
      break;

    case 'T': // Turnout
    {
      uint16_t id;
      if(auto r = fromChars(message.substr(3), id); r.ec == std::errc())
      {
        const char state = *(r.ptr + 1);

        if(state == '0' || state == 'C')
          reply(Ex::setTurnoutResponse(id, false));
        else if(state == '1' || state == 'T')
          reply(Ex::setTurnoutResponse(id, true));
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
          reply(Ex::setOutputResponse(id, false));
        else if(state == '1')
          reply(Ex::setOutputResponse(id, true));
      }
      break;
    }
  }

  return true;
}

void SimulationIOHandler::reply(std::string_view message)
{
  // post the reply, so it has some delay
  //! \todo better delay simulation? at least dcc++ message transfer time?
  m_kernel.ioContext().post(
    [this, data=std::string(message)]()
    {
      m_kernel.receive(data);
    });
}

}
