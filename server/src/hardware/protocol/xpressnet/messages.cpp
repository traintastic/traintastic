/**
 * server/src/hardware/protocol/xpressnet/messages.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2022,2024 Reinder Feenstra
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
#include "../../../utils/tohex.hpp"

namespace XpressNet {

uint8_t calcChecksum(const Message& msg, const int dataSize)
{
  const auto* p = reinterpret_cast<const uint8_t*>(&msg);
  uint8_t checksum = p[0];
  for(int i = 1; i <= dataSize; i++)
    checksum ^= p[i];
  return checksum;
}

void updateChecksum(Message& msg)
{
  *(reinterpret_cast<uint8_t*>(&msg) + msg.dataSize() + 1) = calcChecksum(msg);
}

bool isChecksumValid(const Message& msg, const int dataSize)
{
  return calcChecksum(msg, dataSize) == *(reinterpret_cast<const uint8_t*>(&msg) + dataSize + 1);
}

std::string toString(const Message& message, bool raw, const PendingQuery &pendingQuery)
{
  std::string s = toHex(message.identification());

  // Human readable:
  switch(message.header)
  {
    case STOP_REQUEST:
    {
      if(message == ResumeOperationsRequest())
      {
        s = "RESUME_OPERATIONS_REQUEST";
      }
      else if(message == StopOperationsRequest())
      {
        s = "STOP_OPERATIONS_REQUEST";
      }
      else
        raw = true;
      break;
    }
    case SET_ACCESSORY_OLD:
    {
      const auto& req = static_cast<const AccessoryDecoderOperationRequestOLD&>(message);
      s.append("AccessoryDecoderOperationRequestOLD");
      s.append(" address=").append(std::to_string(req.address()));
      s.append(" port=").append(req.port() ? "2" : "1");
      s.append(req.activate() ? " activate" : " deactivate");
      break;
    }
    case SET_ACCESSORY:
    {
      const auto& req = static_cast<const AccessoryDecoderOperationRequest&>(message);
      s.append("AccessoryDecoderOperationRequest");
      s.append(" address=").append(std::to_string(req.address()));
      s.append(" port=").append(req.port() ? "2" : "1");
      s.append(req.activate() ? " activate" : " deactivate");
      break;
    }
    case BC_HEADER:
    {
      if(message == NormalOperationResumed())
      {
        s = "NORMAL_OPERATIONS_RESUMED";
      }
      else if(message == TrackPowerOff())
      {
        s = "TRACK_POWER_OFF";
      }
      else
        raw = true;
      break;
    }
    case SET_STOP_LOCO:
    {
      if(message == StopAllLocomotivesRequest())
      {
        s = "STOP_ALL_LOCO_REQUEST";
      }
      else
        raw = true;
      break;
    }
    case BC_STOPPED:
    {
      if(message == EmergencyStop())
      {
        s = "EMERGENCY_STOP";
      }
      else
        raw = true;
      break;
    }
    case SET_STOP_LOCO_SINGLE:
    {
      const auto& stopLoco = static_cast<const EmergencyStopLocomotive&>(message);
      s = "EmergencyStopLocomotive";
      s.append(" address=").append(std::to_string(stopLoco.address()));
      if(stopLoco.isLongAddress())
        s.append("/long");
      break;
    }
    case GET_LOCO_INFO:
    {
      const auto& fakeReq = static_cast<const LocomotiveInstruction&>(message);
      switch (fakeReq.identification)
      {
      case 0:
      {
        s = "QueryLocomotive";
        s.append(" address=").append(std::to_string(fakeReq.address()));
        break;
      }
      case idReplyFuncF13F28:
      {
        const auto& funcInfo = static_cast<const FunctionInfoF13F28&>(message);
        s = "FunctionInfoF13F28";
        s.append(" address=");
        if(pendingQuery.address != 0 && pendingQuery.type == PendingQuery::FuncInfoF13F28)
          s.append(std::to_string(pendingQuery.address));
        else
          s.append("ERR!");
        for(uint8_t i = 13; i <= 28; i++)
          s.append(" f").append(std::to_string(i)).append("=").append(funcInfo.getFunction(i) ? "1" : "0");
        break;
      }
      default:
        raw = true;
        break;
      }
      break;
    }
    case SET_LOCO:
    {
      const auto& req = static_cast<const LocomotiveInstruction&>(message);
      switch (req.identification)
      {
      case idLocomotiveBusy:
      {
        s = "LocomotiveBusy";
        s.append(" address=").append(std::to_string(req.address()));
        break;
      }
      case idSetSpeed14:
      case idSetSpeed27:
      case idSetSpeed28:
      case idSetSpeed128:
      {
        const auto& spd = static_cast<const SpeedAndDirectionInstruction&>(message);

        s = "SpeedAndDirectionInstruction" + std::to_string(spd.speedSteps());
        s.append(" address=").append(std::to_string(spd.address()));
        if(spd.isLongAddress())
          s.append("/long");
        s.append(" direction=").append(spd.direction() == Direction::Forward ? "fwd" : "rev");
        s.append(" speed=");
        if(spd.isEmergencyStop())
          s.append("estop");
        else
          s.append(std::to_string(spd.speedStep())).append("/").append(std::to_string(spd.speedSteps()));

        if(spd.identification == idSetSpeed14)
          s.append(" f0=").append(static_cast<const SpeedAndDirectionInstruction14&>(spd).getFl() ? "1" : "0");
        break;
      }
      default:
      {
        const auto& setFunc = static_cast<const FunctionInstructionGroup&>(message);
        const uint8_t funcGroup = setFunc.getGroup();
        if(funcGroup > 0)
        {
          const uint8_t funcMin = FunctionInstructionGroup::getMinFunctionIndex(funcGroup);
          const uint8_t funcMax = FunctionInstructionGroup::getMaxFunctionIndex(funcGroup);

          s = "FunctionInstructionGroup" + std::to_string(funcGroup);
          s.append(" address=").append(std::to_string(setFunc.address()));
          for(uint8_t i = funcMin; i <= funcMax; i++)
            s.append(" f").append(std::to_string(i)).append("=").append(setFunc.getFunction(i) ? "1" : "0");
        }
        else if((req.identification & LocomotiveInfo::identificationMask) == 0)
        {
          const auto& locoInfo = static_cast<const LocomotiveInfo&>(message);
          s = "LocomotiveInfo";
          s.append(" address=");
          if(pendingQuery.address != 0 && pendingQuery.type == PendingQuery::LocoInfoAndF0F12)
            s.append(std::to_string(pendingQuery.address));
          else
            s.append("ERR!");
          s.append(" direction=").append(locoInfo.direction() == Direction::Forward ? "fwd" : "rev");
          s.append(" speed=");
          if(locoInfo.isEmergencyStop())
            s.append("estop");
          else
            s.append(std::to_string(locoInfo.speedStep())).append("/").append(std::to_string(locoInfo.speedSteps()));

          for(uint8_t i = 0; i <= 12; i++)
            s.append(" f").append(std::to_string(i)).append("=").append(locoInfo.getFunction(i) ? "1" : "0");

          s.append(" busy=").append(locoInfo.isBusy() ? "1" : "0");
          break;
        }

        raw = true;
        break;
      }
      }
      break;
    }
    case FUNC_INFO_V4:
    {
      const auto& funcInfo = static_cast<const FunctionInfoF29F68&>(message);
      if(funcInfo.identification == idReplyFuncF29F68)
      {
        s = "FunctionInfoF29F68";
        s.append(" address=");
        if(pendingQuery.address != 0 && pendingQuery.type == PendingQuery::FuncInfoF29F68)
          s.append(std::to_string(pendingQuery.address));
        else
          s.append("ERR!");
        for(uint8_t i = 29; i <= 68; i++)
          s.append(" f").append(std::to_string(i)).append("=").append(funcInfo.getFunction(i) ? "1" : "0");
        break;
      }

      raw = true;
      break;
    }
    default:
    {
      raw = true;
      break;
    }
    // FIXME: add all messages
  }

  if(raw)
  {
    // Raw data:
    s.append(" [");
    s.append(toHex(reinterpret_cast<const uint8_t*>(&message), message.size(), true));
    s.append("]");
  }

  return s;
}

}
