/**
 * server/src/hardware/protocol/marklincan/messages.cpp
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

#include "messages.hpp"
#include "uid.hpp"
#include "../../../utils/tohex.hpp"

namespace MarklinCAN {

std::string toString(const Message& message)
{
  std::string s;

  switch(message.command())
  {
    case Command::System:
    {
      s.append("System");

      const auto& system = static_cast<const SystemMessage&>(message);

      switch(system.subCommand())
      {
        case SystemSubCommand::SystemStop:
          s.append(" stop");
          break;

        case SystemSubCommand::SystemGo:
          s.append(" go");
          break;

        case SystemSubCommand::SystemHalt:
          s.append(" halt");
          break;

        case SystemSubCommand::LocomotiveEmergencyStop:
          break;

        case SystemSubCommand::LocomotiveCycleEnd:
          break;

        case SystemSubCommand::Overload:
          break;

        case SystemSubCommand::Status:
          break;
      }
      break;
    }
    case Command::Discovery:
      s.append("Discovery");
      break;

    case Command::Bind:
      s.append("Bind");
      break;

    case Command::Verify:
      s.append("Verify");
      break;

    case Command::LocomotiveSpeed:
    {
      s.append("LocomotiveSpeed");

      const auto& locomotiveSpeed = static_cast<const LocomotiveSpeed&>(message);

      if(!locomotiveSpeed.hasSpeed() && !locomotiveSpeed.isResponse())
        s.append(" get");
      else if(locomotiveSpeed.hasSpeed() && !locomotiveSpeed.isResponse())
        s.append(" set");
      else if(locomotiveSpeed.hasSpeed() && locomotiveSpeed.isResponse())
        s.append(" ack");
      else
        s.append(" ???");

      s.append(" ").append(UID::toString(locomotiveSpeed.uid()));

      if(locomotiveSpeed.dlc == 6)
        s.append(" speed=").append(std::to_string(locomotiveSpeed.speed()));

      break;
    }
    case Command::LocomotiveDirection:
    {
      s.append("LocomotiveDirection");

      const auto& locomotiveDirection = static_cast<const LocomotiveDirection&>(message);

      if(!locomotiveDirection.hasDirection() && !locomotiveDirection.isResponse())
        s.append(" get");
      else if(locomotiveDirection.hasDirection() && !locomotiveDirection.isResponse())
        s.append(" set");
      else if(locomotiveDirection.hasDirection() && locomotiveDirection.isResponse())
        s.append(" ack");
      else
        s.append(" ???");

      s.append(" ").append(UID::toString(locomotiveDirection.uid()));

      if(locomotiveDirection.hasDirection())
      {
        s.append(" direction=");

        switch(locomotiveDirection.direction())
        {
          case LocomotiveDirection::Direction::Same:
            s.append("same");
            break;

          case LocomotiveDirection::Direction::Forward:
            s.append("forward");
            break;

          case LocomotiveDirection::Direction::Reverse:
            s.append("reverse");
            break;

          case LocomotiveDirection::Direction::Inverse:
            s.append("inverse");
            break;
        }
      }
      break;
    }
    case Command::LocomotiveFunction:
    {
      s.append("LocomotiveFunction");

      const auto& locomotiveFunction = static_cast<const LocomotiveFunction&>(message);

      if(!locomotiveFunction.hasValue() && !locomotiveFunction.isResponse())
        s.append(" get");
      else if(locomotiveFunction.hasValue() && !locomotiveFunction.isResponse())
        s.append(" set");
      else if(locomotiveFunction.hasValue() && locomotiveFunction.isResponse())
        s.append(" ack");
      else
        s.append(" ???");

      s.append(" ").append(UID::toString(locomotiveFunction.uid()));

      s.append(" number=").append(std::to_string(locomotiveFunction.number()));

      if(locomotiveFunction.hasValue())
        s.append(" value=").append(std::to_string(locomotiveFunction.value()));

      break;
    }
    case Command::ReadConfig:
      s.append("ReadConfig");
      break;

    case Command::WriteConfig:
      s.append("WriteConfig");
      break;

    case Command::AccessoryControl:
      s.append("AccessoryControl");
      break;

    case Command::AccessoryConfig:
      s.append("AccessoryConfig");
      break;

    case Command::S88Polling:
      s.append("S88Polling");
      break;

    case Command::FeedbackEvent:
    {
      s.append("FeedbackEvent");

      const auto& feedbackEvent = static_cast<const FeedbackMessage&>(message);
      s.append(" device_id=").append(std::to_string(feedbackEvent.deviceId()));
      s.append(" contact_id=").append(std::to_string(feedbackEvent.contactId()));

      if(message.dlc == 5)
      {
        const auto& feedbackStateParameter = static_cast<const FeedbackStateParameter&>(message);
        s.append(" parameter=").append(std::to_string(feedbackStateParameter.parameter()));
      }
      else if(message.dlc == 8)
      {
        const auto& feedbackState = static_cast<const FeedbackState&>(message);
        s.append(" state_old=").append(std::to_string(feedbackState.stateOld()));
        s.append(" state_new=").append(std::to_string(feedbackState.stateNew()));
        s.append(" time=").append(std::to_string(feedbackState.time()));
      }
      break;
    }
    case Command::SX1Event:
      s.append("SX1Event");
      break;

    case Command::Ping:
      s.append("Ping");
      break;

    case Command::Update:
      s.append("Update");
      break;

    case Command::ReadConfigData:
      s.append("ReadConfigData");
      break;

    case Command::BootloaderCAN:
      s.append("BootloaderCAN");
      break;

    case Command::BootloaderTrack:
      s.append("BootloaderTrack");
      break;

    case Command::StatusDataConfig:
      s.append("StatusDataConfig");
      break;

    case Command::ConfigData:
      s.append("ConfigData");
      break;

    case Command::ConfigDataStream:
      s.append("ConfigDataStream");
      break;
  }

  // raw:
  s.append(s.empty() ? "[" : " [").append(std::to_string(message.priority()));
  s.append(" ").append(toHex(static_cast<uint8_t>(message.command())));
  s.append(message.isResponse() ? " 1" : " 0");
  s.append(" ").append(toHex(message.hash()));
  s.append(" ").append(std::to_string(message.dlc));
  if(message.dlc > 0)
    s.append(" ").append(toHex(message.data, message.dlc));
  s.append("]");

  return s;
}

}
