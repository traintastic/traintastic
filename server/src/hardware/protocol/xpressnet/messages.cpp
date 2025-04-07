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

std::string toString(const Message& message, bool raw)
{
  std::string s = toHex(message.identification());

  // Human readable:
  switch(message.header)
  {
    case 0x21:
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
    case 0x61:
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
    case 0x80:
    {
      if(message == StopAllLocomotivesRequest())
      {
        s = "STOP_ALL_LOCO_REQUEST";
      }
      else
        raw = true;
      break;
    }
    case 0x81:
    {
      if(message == EmergencyStop())
      {
        s = "EMERGENCY_STOP";
      }
      else
        raw = true;
      break;
    }
    case 0x52:
    {
      const auto& req = static_cast<const AccessoryDecoderOperationRequest&>(message);
      s.append("AccessoryDecoderOperationRequest");
      s.append(" address=").append(std::to_string(req.address()));
      s.append(" port=").append(req.port() ? "2" : "1");
      s.append(req.activate() ? " activate" : " deactivate");
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
