/**
 * server/src/hardware/protocol/dinamo/iohandler/simulationiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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
#include <cstddef>
#include "../kernel.hpp"
#include "../messages.hpp"

namespace Dinamo {

static std::shared_ptr<std::byte[]> copy(const Message& message)
{
  auto* bytes = new std::byte[message.size()];
  std::memcpy(bytes, &message, message.size());
  return std::shared_ptr<std::byte[]>{bytes};
}

SimulationIOHandler::SimulationIOHandler(Kernel& kernel)
  : IOHandler(kernel)
{
}

void SimulationIOHandler::start()
{
  m_kernel.started();
}

bool SimulationIOHandler::send(const Message& message)
{
  if(message.header.dataSize() >= 1)
  {
    switch(static_cast<const Command&>(message).command)
    {
      case Command::systemControl:
        if(message.header.dataSize() >= 2)
        {
          switch(static_cast<const SubCommand&>(message).subCommand)
          {
            case SubCommand::resetFault:
              m_fault = false;
              break;

            case SubCommand::protocolVersion:
            {
              if(message.size() == sizeof(ProtocolVersionRequest))
              {
                ProtocolVersionResponse response(3, 1, 2, 1);
                reply(response);
              }
              break;
            }
          }
        }
        break;
    }
  }

  return true;
}

void SimulationIOHandler::reply(Message& message)
{
  message.header.setToggle(m_replyToggle);
  m_replyToggle = !m_replyToggle;
  if(!message.header.isJumbo())
  {
    message.header.setFault(m_fault);
  }
  updateChecksum(message);

  // post the reply, so it has some delay
  //! \todo better delay simulation? at least message transfer time?
  m_kernel.ioContext().post(
    [this, data=copy(message)]()
    {
      m_kernel.receive(*reinterpret_cast<const Message*>(data.get()));
    });
}

}
