/**
 * server/src/hardware/protocol/z21/messages.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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
#include "../../decoder/decoder.hpp"
#include "../../../utils/to_hex.hpp"

namespace Z21 {

static std::string_view to_string(Header header)
{
  switch(header)
  {
    case LAN_GET_SERIAL_NUMBER: return "LAN_GET_SERIAL_NUMBER";
    case LAN_GET_HWINFO: return "LAN_GET_HWINFO";
    case LAN_LOGOFF: return "LAN_LOGOFF";
    case LAN_X: return "LAN_X";
    case LAN_SET_BROADCASTFLAGS: return "LAN_SET_BROADCASTFLAGS";
    case LAN_GET_BROADCASTFLAGS: return "LAN_GET_BROADCASTFLAGS";
    case LAN_SYSTEMSTATE_DATACHANGED: return "LAN_SYSTEMSTATE_DATACHANGED";
    case LAN_SYSTEMSTATE_GETDATA: return "LAN_SYSTEMSTATE_GETDATA";
    case LAN_LOCONET_Z21_RX: return "LAN_LOCONET_Z21_RX";
    case LAN_LOCONET_Z21_TX: return "LAN_LOCONET_Z21_TX";
  }
  return {};
}

std::string to_string(const Message& message, bool raw)
{
  std::string s;
  if(std::string_view sv = to_string(message.header()); !sv.empty())
    s = sv;
  else
    s = to_hex(message.header());

  switch(message.header())
  {
    case LAN_LOGOFF:
      if(message.dataLen() != sizeof(Z21::LanLogoff))
        raw = true;
      break;

    case LAN_X:
      switch(static_cast<const LanX&>(message).xheader)
      {
        case 0x21:
          if(message == LanXGetStatus())
            s = "LAN_X_GET_STATUS";
          else
            raw = true;
          break;

        case 0x62:
          if(const LanXStatusChanged& statusChanged = static_cast<const LanXStatusChanged&>(message); statusChanged.db0 == 0x22)
          {
            s = "LAN_X_STATUS_CHANGED";
            s.append(" emergency_stop=").append(statusChanged.db1 & Z21_CENTRALSTATE_EMERGENCYSTOP ? "yes" : "no");
            s.append(" track_voltage=").append(statusChanged.db1 & Z21_CENTRALSTATE_TRACKVOLTAGEOFF ? "off" : "on");
            s.append(" short_circuit=").append(statusChanged.db1 & Z21_CENTRALSTATE_SHORTCIRCUIT ? "yes" : "no");
            s.append(" programming_mode_active=").append(statusChanged.db1 & Z21_CENTRALSTATE_PROGRAMMINGMODEACTIVE ? "yes" : "no");
          }
          else
            raw = true;
          break;

        default:
          raw = true;
          break;
      }
      break;

    case Z21::LAN_GET_BROADCASTFLAGS:
      if(message.dataLen() == sizeof(Z21::LanGetBroadcastFlags))
        s = "LAN_GET_BROADCASTFLAGS";
      //else if(message.dataLen() == sizeof(Z21::LanGetBroadcastFlagsReply))
      //  s = "LAN_GET_BROADCASTFLAGS flags=0x" + to_hex(static_cast<const LanGetBroadcastFlagsReply&>(message).broadcastFlags()));
      else
        raw = true;
      break;

    case Z21::LAN_SET_BROADCASTFLAGS:
      if(message.dataLen() == sizeof(Z21::LanSetBroadcastFlags))
        s = "LAN_SET_BROADCASTFLAGS flags=0x" + to_hex(static_cast<const LanSetBroadcastFlags&>(message).broadcastFlags());
      else
        raw = true;
      break;

    default:
      raw = true;
      break;
  }

  if(raw)
  {
    s.append(" [");
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&message);
    for(uint16_t i = sizeof(Message); i < message.dataLen(); i++)
    {
      if(i != sizeof(Message))
        s.append(" ");
      s.append(to_hex(bytes[i]));
    }
    s.append("]");
  }

  return s;
}

LanXLocoInfo::LanXLocoInfo(const Decoder& decoder) :
  LanXLocoInfo()
{
  setAddress(decoder.address, decoder.longAddress);
  setSpeedSteps(decoder.speedSteps);
  setDirection(decoder.direction);
  if(decoder.emergencyStop)
    setEmergencyStop();
  else
    setSpeedStep(decoder.speedStep);
  for(auto function : *decoder.functions)
    setFunction(function->number, function->value);
  calcChecksum();
}

}