/**
 * server/src/hardware/protocol/z21/messages.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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
#include "../../../core/objectproperty.tpp"
#include "../../../utils/tohex.hpp"

namespace Z21 {

static std::string_view toString(Header header)
{
  switch(header)
  {
    case LAN_GET_SERIAL_NUMBER: return "LAN_GET_SERIAL_NUMBER";
    case LAN_GET_CODE: return "LAN_GET_CODE";
    case LAN_GET_HWINFO: return "LAN_GET_HWINFO";
    case LAN_LOGOFF: return "LAN_LOGOFF";
    case LAN_X: return "LAN_X";
    case LAN_SET_BROADCASTFLAGS: return "LAN_SET_BROADCASTFLAGS";
    case LAN_GET_BROADCASTFLAGS: return "LAN_GET_BROADCASTFLAGS";
    case LAN_GET_LOCO_MODE: return "LAN_GET_LOCO_MODE";
    case LAN_SET_LOCO_MODE: return "LAN_SET_LOCO_MODE";
    case LAN_GET_TURNOUTMODE: return "LAN_GET_TURNOUTMODE";
    case LAN_SET_TURNOUTMODE: return "LAN_SET_TURNOUTMODE";
    case LAN_RMBUS_DATACHANGED: return "LAN_RMBUS_DATACHANGED";
    case LAN_RMBUS_GETDATA: return "LAN_RMBUS_GETDATA";
    case LAN_RMBUS_PROGRAMMODULE: return "LAN_RMBUS_PROGRAMMODULE";
    case LAN_SYSTEMSTATE_DATACHANGED: return "LAN_SYSTEMSTATE_DATACHANGED";
    case LAN_SYSTEMSTATE_GETDATA: return "LAN_SYSTEMSTATE_GETDATA";
    case LAN_RAILCOM_DATACHANGED: return "LAN_RAILCOM_DATACHANGED";
    case LAN_RAILCOM_GETDATA: return "LAN_RAILCOM_GETDATA";
    case LAN_LOCONET_Z21_RX: return "LAN_LOCONET_Z21_RX";
    case LAN_LOCONET_Z21_TX: return "LAN_LOCONET_Z21_TX";
    case LAN_LOCONET_FROM_LAN: return "LAN_LOCONET_FROM_LAN";
    case LAN_LOCONET_DISPATCH_ADDR: return "LAN_LOCONET_DISPATCH_ADDR";
    case LAN_LOCONET_DETECTOR: return "LAN_LOCONET_DETECTOR";
    case LAN_CAN_DETECTOR: return "LAN_CAN_DETECTOR";
  }
  return {};
}

std::string toString(const Message& message, bool raw)
{
  std::string s;
  if(std::string_view sv = toString(message.header()); !sv.empty())
    s = sv;
  else
    s = toHex(message.header());

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
          if(message == LanXGetVersion())
            s = "LAN_X_GET_VERSION";
          else if(message == LanXGetStatus())
            s = "LAN_X_GET_STATUS";
          else if(message == LanXSetTrackPowerOff())
            s = "LAN_X_SET_TRACK_POWER_OFF";
          else if(message == LanXSetTrackPowerOn())
            s = "LAN_X_SET_TRACK_POWER_ON";
          else
            raw = true;
          break;

        case 0x43:
        {
          const auto& getTurnoutInfo = static_cast<const LanXGetTurnoutInfo&>(message);
          s = "LAN_X_GET_TURNOUT_INFO";
          s.append(" address=").append(std::to_string(getTurnoutInfo.address()));
          break;
        }
        case 0x53:
        {
          const auto& setTurnout = static_cast<const LanXSetTurnout&>(message);
          s = "LAN_X_SET_TURNOUT";
          s.append(" linear_address=").append(std::to_string(setTurnout.linearAddress()));
          s.append(" address=").append(std::to_string(setTurnout.address()));
          s.append(" port=").append(std::to_string(setTurnout.port()));
          s.append(" activate=").append(setTurnout.activate() ? "yes" : "no");
          s.append(" queue=").append(setTurnout.queue() ? "yes" : "no");
          break;
        }
        case 0x61:
          if(message == LanXBCTrackPowerOff())
            s = "LAN_X_BC_TRACK_POWER_OFF";
          else if(message == LanXBCTrackPowerOn())
            s = "LAN_X_BC_TRACK_POWER_ON";
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

        case 0x80:
          if(message == LanXSetStop())
            s = "LAN_X_SET_STOP";
          else
            raw = true;
          break;

        case 0xE3:
          if(const auto& getLocoInfo = static_cast<const LanXGetLocoInfo&>(message); getLocoInfo.db0 == 0xF0)
          {
            s = "LAN_X_GET_LOCO_INFO";
            s.append(" address=").append(std::to_string(getLocoInfo.address()));
            if(getLocoInfo.isLongAddress())
              s.append(" (long)");
          }
          else
            raw = true;
          break;

        case 0xE4:
          if(const auto& setLocoDrive = static_cast<const LanXSetLocoDrive&>(message);
              setLocoDrive.db0 >= 0x10 && setLocoDrive.db0 <= 0x13)
          {
            s = "LAN_X_SET_LOCO_DRIVE";
            s.append(" address=").append(std::to_string(setLocoDrive.address()));
            if(setLocoDrive.isLongAddress())
              s.append("/long");
            s.append(" direction=").append(setLocoDrive.direction() == Direction::Forward ? "fwd" : "rev");
            s.append(" speed=");
            if(setLocoDrive.isEmergencyStop())
              s.append("estop");
            else
              s.append(std::to_string(setLocoDrive.speedStep())).append("/").append(std::to_string(setLocoDrive.speedSteps()));
          }
          else if(const auto& setLocoFunction = static_cast<const LanXSetLocoFunction&>(message);
              setLocoFunction.db0 == 0xF8)
          {
            s = "LAN_X_SET_LOCO_FUNCTION";
            s.append(" address=").append(std::to_string(setLocoFunction.address()));
            if(setLocoFunction.isLongAddress())
              s.append("/long");
            s.append(" function=").append(std::to_string(setLocoFunction.functionIndex()));
            s.append(" state=").append(toString(setLocoFunction.switchType()));
          }
          else
            raw = true;
          break;

        case 0xEF:
        {
          const auto& locoInfo = static_cast<const LanXLocoInfo&>(message);
          s = "LAN_X_LOCO_INFO";
          s.append(" address=").append(std::to_string(locoInfo.address()));
          if(locoInfo.isLongAddress())
            s.append("/long");
          s.append(" direction=").append(locoInfo.direction() == Direction::Forward ? "fwd" : "rev");
          s.append(" speed=");
          if(locoInfo.isEmergencyStop())
            s.append("estop");
          else
            s.append(std::to_string(locoInfo.speedStep())).append("/").append(std::to_string(locoInfo.speedSteps()));
          for(uint8_t i = 0; i <= LanXLocoInfo::functionIndexMax; i++)
            s.append(" f").append(std::to_string(i)).append("=").append(locoInfo.getFunction(i) ? "1" : "0");
          s.append(" busy=").append(locoInfo.isBusy() ? "1" : "0");
          break;
        }
        case 0xF1:
          if(message == LanXGetFirmwareVersion())
            s = "LAN_X_GET_FIRMWARE_VERSION";
          else
            raw = true;
          break;

        case 0xF3:
          if(message.dataLen() == sizeof(LanXGetFirmwareVersionReply))
          {
            const auto& getFirmwareVersion = static_cast<const LanXGetFirmwareVersionReply&>(message);
            s = "LAN_X_GET_FIRMWARE_VERSION";
            s.append(" version=").append(std::to_string(getFirmwareVersion.versionMajor())).append(".").append(std::to_string(getFirmwareVersion.versionMinor()));
          }
          else
            raw = true;
          break;

        default:
          raw = true;
          break;
      }
      break;

    case LAN_GET_BROADCASTFLAGS:
      if(message == LanGetBroadcastFlags())
        s = "LAN_GET_BROADCASTFLAGS";
      else
        raw = true;
      break;

    case LAN_SET_BROADCASTFLAGS:
      if(message.dataLen() == sizeof(Z21::LanSetBroadcastFlags))
      {
        s = "LAN_SET_BROADCASTFLAGS";
        s.append(" flags=0x").append(toHex(static_cast<std::underlying_type_t<BroadcastFlags>>(static_cast<const LanSetBroadcastFlags&>(message).broadcastFlags())));
      }
      else
        raw = true;
      break;

    case LAN_SYSTEMSTATE_DATACHANGED:
    {
      const auto& systemState = static_cast<const LanSystemStateDataChanged&>(message);
      s.append(" mainCurrent=").append(std::to_string(systemState.mainCurrent));
      s.append(" progCurrent=").append(std::to_string(systemState.progCurrent));
      s.append(" filteredMainCurrent=").append(std::to_string(systemState.filteredMainCurrent));
      s.append(" temperature=").append(std::to_string(systemState.temperature));
      s.append(" supplyVoltage=").append(std::to_string(systemState.supplyVoltage));
      s.append(" vccVoltage=").append(std::to_string(systemState.vccVoltage));
      s.append(" centralState=0x").append(toHex(systemState.centralState));
      s.append(" centralStateEx=0x").append(toHex(systemState.centralStateEx));
      break;
    }
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
      s.append(toHex(bytes[i]));
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
    setSpeedStep(Decoder::throttleToSpeedStep(decoder.throttle, speedSteps()));
  for(auto function : *decoder.functions)
    setFunction(function->number, function->value);
  calcChecksum();
}

}