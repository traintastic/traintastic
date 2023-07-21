/**
 * server/src/hardware/protocol/marklincan/iohandler/simulationiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

namespace MarklinCAN {

SimulationIOHandler::SimulationIOHandler(Kernel& kernel)
  : IOHandler(kernel)
{
}

bool SimulationIOHandler::send(const Message& message)
{
  switch(message.command())
  {
    case Command::System:
    {
      const auto& system = static_cast<const SystemMessage&>(message);

      switch(system.subCommand())
      {
        case SystemSubCommand::SystemStop:
        case SystemSubCommand::SystemGo:
        case SystemSubCommand::SystemHalt:
        case SystemSubCommand::LocomotiveEmergencyStop:
        case SystemSubCommand::LocomotiveCycleEnd:
          // not (yet) implemented
          break;

        case SystemSubCommand::AccessorySwitchTime:
          if(!message.isResponse() && message.dlc == 7)
          {
            auto response = static_cast<const AccessorySwitchTime&>(message);
            m_switchTime = std::chrono::milliseconds(response.switchTime() * 10);
            response.setResponse(true);
            reply(response);
          }
          break;

        case SystemSubCommand::Overload:
        case SystemSubCommand::Status:
          // not (yet) implemented
          break;
      }
      break;
    }
    case Command::Discovery:
    case Command::Bind:
    case Command::Verify:
    case Command::LocomotiveSpeed:
    case Command::LocomotiveDirection:
    case Command::LocomotiveFunction:
    case Command::ReadConfig:
    case Command::WriteConfig:
    case Command::AccessoryControl:
    case Command::AccessoryConfig:
    case Command::S88Polling:
    case Command::FeedbackEvent:
    case Command::SX1Event:
    case Command::Ping:
    case Command::Update:
    case Command::ReadConfigData:
    case Command::BootloaderCAN:
    case Command::BootloaderTrack:
    case Command::StatusDataConfig:
    case Command::ConfigData:
    case Command::ConfigDataStream:
      // not (yet) implemented
      break;
  }
  return true;
}

void SimulationIOHandler::reply(const Message& message)
{
  // post the reply, so it has some delay
  m_kernel.ioContext().post(
    [this, message]()
    {
      m_kernel.receive(message);
    });
}

}
