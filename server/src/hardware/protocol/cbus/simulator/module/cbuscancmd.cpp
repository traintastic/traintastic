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

#include "cbuscancmd.hpp"
#include <algorithm>
#include "../../cbusmessages.hpp"

namespace CBUS::Module {

CANCMD::CANCMD(Simulator& simulator, uint16_t nodeNumber_, uint8_t canId_)
  : CANModule(simulator, nodeNumber_, canId_)
{
}

void CANCMD::receive(const Message& message)
{
  switch(message.opCode)
  {
    using enum OpCode;

    case RTOF:
      m_trackOn = false;
      send(TrackOff());
      break;

    case RTON:
      m_trackOn = true;
      send(TrackOn());
      break;

    case RLOC:
    {
      const auto& rloc = static_cast<const RequestEngineSession&>(message);
      handleGetEngineSession(rloc.address(), rloc.isLongAddress(), GetEngineSession::Mode::Request);
      break;
    }
    case GLOC:
    {
      const auto& gloc = static_cast<const GetEngineSession&>(message);
      handleGetEngineSession(gloc.address(), gloc.isLongAddress(), gloc.mode());
      break;
    }
    default:
      break;
  }
}

EngineReport* CANCMD::newEngineSession(const EngineReport& init)
{
  for(uint8_t session = 0;; ++session)
  {
    if(!m_sessions.contains(session))
    {
      auto& newSession = m_sessions.emplace(session, init).first->second;
      newSession.session = session;
      return &newSession;
    }
    if (session == 255)
    {
      break;
    }
  }
  return nullptr;
}

void CANCMD::handleGetEngineSession(uint16_t address, bool isLongAddress, GetEngineSession::Mode mode)
{
  using enum GetEngineSession::Mode;

  if(mode == static_cast<GetEngineSession::Mode>(0b11))
  {
    send(CommandStationInvalidRequestError(address, isLongAddress));
    return;
  }

  if(auto it = std::find_if(m_sessions.begin(), m_sessions.end(),
    [address, isLongAddress](const auto& item)
    {
      return item.second.address() == address && item.second.isLongAddress() == isLongAddress;
    }); it != m_sessions.end()) // existing session
  {
    if(mode == Request)
    {
      send(CommandStationLocoAddressTakenError(address, isLongAddress));
    }
    else if(mode == Share)
    {
      send(it->second);
    }
    else // Steal
    {
      if(auto* ploc = newEngineSession(it->second))
      {
        const auto oldSession = it->first;
        send(CommandStationSessionCancelled(oldSession));
        send(*ploc);
        m_sessions.erase(oldSession);
      }
      else
      {
        send(CommandStationLocoStackFullError(address, isLongAddress));
      }
    }
  }
  else // new session
  {
    if(mode == Share)
    {
      // FIXME: If there is no current session for a share then send an ERR(No session) message.
    }
    else // Release or Steal
    {
      if(const auto* ploc = newEngineSession(EngineReport(
          0,
          address, isLongAddress,
          0, true,
          false, false, false, false, false,
          false, false, false, false,
          false, false, false, false)))
      {
        send(*ploc);
      }
      else
      {
        send(CommandStationLocoStackFullError(address, isLongAddress));
      }
    }
  }
}

}
