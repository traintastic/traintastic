/**
 * server/src/hardware/protocol/z21/iohandler/simulationiohandler.cpp
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
#include "../clientkernel.hpp"
#include "../messages.hpp"

namespace Z21 {

static std::shared_ptr<std::byte[]> copy(const Message& message)
{
  auto* bytes = new std::byte[message.dataLen()];
  std::memcpy(bytes, &message, message.dataLen());
  return std::shared_ptr<std::byte[]>{bytes};
}

SimulationIOHandler::SimulationIOHandler(Kernel& kernel)
  : IOHandler(kernel)
{
}

bool SimulationIOHandler::send(const Message& message)
{
  switch(message.header())
  {
    case LAN_X:
    {
      const auto& lanX = static_cast<const LanX&>(message);

      switch(lanX.xheader)
      {
        case 0x21:
        {
          if(message == LanXGetVersion())
          {
            reply(LanXGetVersionReply(xBusVersion, CommandStationId::Z21));
          }
          else if(message == LanXGetStatus())
          {
            LanXStatusChanged response;
            if(m_emergencyStop)
              response.db1 |= Z21_CENTRALSTATE_EMERGENCYSTOP;
            if(!m_trackPowerOn)
              response.db1 |= Z21_CENTRALSTATE_TRACKVOLTAGEOFF;
            response.updateChecksum();
            reply(response);
          }
          else if(message == LanXSetTrackPowerOn())
          {
            const bool changed = !m_trackPowerOn || m_emergencyStop;
            m_trackPowerOn = true;
            m_emergencyStop = false;
            reply(LanXBCTrackPowerOn());
            if(changed && (m_broadcastFlags & BroadcastFlags::SystemStatusChanges) == BroadcastFlags::SystemStatusChanges)
            {
              replyLanSystemStateDataChanged();
            }
          }
          else if(message == LanXSetTrackPowerOff())
          {
            const bool changed = m_trackPowerOn;
            m_trackPowerOn = false;
            reply(LanXBCTrackPowerOff());
            if(changed && (m_broadcastFlags & BroadcastFlags::SystemStatusChanges) == BroadcastFlags::SystemStatusChanges)
            {
              replyLanSystemStateDataChanged();
            }
          }
          break;
        }
        case LAN_X_SET_STOP:
        {
          if(message == LanXSetStop())
          {
            const bool changed = !m_emergencyStop;
            m_emergencyStop = true;
            reply(LanXBCStopped());
            if(changed && (m_broadcastFlags & BroadcastFlags::SystemStatusChanges) == BroadcastFlags::SystemStatusChanges)
            {
              replyLanSystemStateDataChanged();
            }
          }
          break;
        }
        case LAN_X_GET_LOCO_INFO:
        {
          if(const auto& getLocoInfo = static_cast<const LanXGetLocoInfo&>(message);
              getLocoInfo.db0 == 0xF0)
          {
            auto it = m_decoderCache.find(getLocoInfo.address());
            if(it != m_decoderCache.cend())
              reply(it->second);
            else
            {
              LanXLocoInfo empty;
              empty.setAddress(getLocoInfo.address(), getLocoInfo.isLongAddress());
              empty.setSpeedSteps(126);
              empty.setEmergencyStop();
              empty.updateChecksum();
              reply(empty);
            }
          }
          break;
        }
        case LAN_X_SET_LOCO:
        {
          if(const auto& setLocoDrive = static_cast<const LanXSetLocoDrive&>(message);
              setLocoDrive.db0 >= 0x10 && setLocoDrive.db0 <= 0x13)
          {
            auto it = m_decoderCache.find(setLocoDrive.address());
            if(it == m_decoderCache.cend())
            {
              // Insert in cache
              LanXLocoInfo empty;
              empty.setAddress(setLocoDrive.address(), setLocoDrive.isLongAddress());
              empty.setSpeedSteps(126);
              empty.setEmergencyStop();
              it = m_decoderCache.insert({setLocoDrive.address(), empty}).first;
            }

            LanXLocoInfo &info = it->second;
            info.setSpeedSteps(setLocoDrive.speedSteps());
            info.setDirection(setLocoDrive.direction());
            if(setLocoDrive.isEmergencyStop())
              info.setEmergencyStop();
            else
              info.setSpeedStep(setLocoDrive.speedStep());

            info.setBusy(true);
            info.updateChecksum();

            reply(info);
          }
          else if(const auto& setLocoFunction = static_cast<const LanXSetLocoFunction&>(message);
                  setLocoFunction.db0 == 0xF8 &&
                  setLocoFunction.switchType() != LanXSetLocoFunction::SwitchType::Invalid)
          {
            auto it = m_decoderCache.find(setLocoDrive.address());
            if(it == m_decoderCache.cend())
            {
              // Insert in cache
                LanXLocoInfo empty;
                empty.setAddress(setLocoFunction.address(), setLocoFunction.isLongAddress());
                empty.setSpeedSteps(126);
                empty.setEmergencyStop();
                it = m_decoderCache.insert({setLocoFunction.address(), empty}).first;
            }

            LanXLocoInfo &info = it->second;
            bool val = info.getFunction(setLocoFunction.functionIndex());
            switch (setLocoFunction.switchType())
            {
            case LanXSetLocoFunction::SwitchType::Off:
              val = false;
              break;
            case LanXSetLocoFunction::SwitchType::On:
              val = true;
              break;
            case LanXSetLocoFunction::SwitchType::Toggle:
              val = !val;
              break;
            default:
              break;
            }
            info.setFunction(setLocoFunction.functionIndex(), val);

            info.setBusy(true);
            info.updateChecksum();

            reply(info);
          }
          break;
        }
        case LAN_X_GET_FIRMWARE_VERSION:
        {
          if(message == LanXGetFirmwareVersion())
          {
            reply(LanXGetFirmwareVersionReply(firmwareVersionMajor, ServerConfig::firmwareVersionMinor));
          }
          break;
        }
        case LAN_X_SET_TURNOUT:
        {
          if(message.dataLen() == sizeof(LanXSetTurnout))
          {
            const auto& setTurnout = static_cast<const LanXSetTurnout&>(message);
            if((m_broadcastFlags & BroadcastFlags::PowerLocoTurnoutChanges) == BroadcastFlags::PowerLocoTurnoutChanges)
            {
              // Client has subscribed to turnout changes
              reply(LanXTurnoutInfo(setTurnout.address(), setTurnout.port(), false));
            }
          }
          break;
        }
        case LAN_X_TURNOUT_INFO:
        {
          if(message.dataLen() == sizeof(LanXGetTurnoutInfo))
          {
            const auto& getTurnout = static_cast<const LanXGetTurnoutInfo&>(message);
            //We do not keep a record of turnout states so send "Unknown Position"
            reply(LanXTurnoutInfo(getTurnout.address(), false, true));
          }
          break;
        }
        case LAN_X_SET_EXT_ACCESSORY:
        {
          if(message.dataLen() == sizeof(LanXSetExtAccessory))
          {
            const auto& setAccessory = static_cast<const LanXSetExtAccessory&>(message);
            if((m_broadcastFlags & BroadcastFlags::PowerLocoTurnoutChanges) == BroadcastFlags::PowerLocoTurnoutChanges)
            {
              // Client has subscribed to turnout changes
              reply(LanXExtAccessoryInfo(setAccessory.address(), setAccessory.aspect(), false));
            }
          }
          break;
        }
        case LAN_X_EXT_ACCESSORY_INFO:
        {
          if(message.dataLen() == sizeof(LanXGetExtAccessoryInfo))
          {
            const auto& getAccessory = static_cast<const LanXGetExtAccessoryInfo&>(message);
            //We do not keep a record of accessory states so send "Unknown Position"
            reply(LanXExtAccessoryInfo(getAccessory.address(), 0, true));
          }
          break;
        }
      }
      break;
    }
    case LAN_GET_SERIAL_NUMBER:
      if(message.dataLen() == sizeof(LanGetSerialNumber))
      {
        reply(LanGetSerialNumberReply(serialNumber));
      }
      break;

    case LAN_GET_HWINFO:
      if(message.dataLen() == sizeof(LanGetHardwareInfo))
      {
        reply(LanGetHardwareInfoReply(hardwareType, firmwareVersionMajor, firmwareVersionMinor));
      }
      break;

    case LAN_GET_BROADCASTFLAGS:
      if(message == LanGetBroadcastFlags())
      {
        reply(LanGetBroadcastFlagsReply(m_broadcastFlags));
      }
      break;

    case LAN_SET_BROADCASTFLAGS:
      if(message.dataLen() == sizeof(LanSetBroadcastFlags))
      {
        m_broadcastFlags = static_cast<const LanSetBroadcastFlags&>(message).broadcastFlags();
      }
      break;

    case LAN_SYSTEMSTATE_GETDATA:
      if(message == LanSystemStateGetData())
      {
        replyLanSystemStateDataChanged();
      }
      break;

    case LAN_LOGOFF:
    case LAN_GET_CODE:
    case LAN_GET_LOCO_MODE:
    case LAN_SET_LOCO_MODE:
    case LAN_GET_TURNOUTMODE:
    case LAN_SET_TURNOUTMODE:
    case LAN_RMBUS_DATACHANGED:
    case LAN_RMBUS_GETDATA:
    case LAN_RMBUS_PROGRAMMODULE:
    case LAN_SYSTEMSTATE_DATACHANGED:
    case LAN_RAILCOM_DATACHANGED:
    case LAN_RAILCOM_GETDATA:
    case LAN_LOCONET_Z21_RX:
    case LAN_LOCONET_Z21_TX:
    case LAN_LOCONET_FROM_LAN:
    case LAN_LOCONET_DISPATCH_ADDR:
    case LAN_LOCONET_DETECTOR:
    case LAN_CAN_DETECTOR:
      break; // not (yet) supported
  }

  return true;
}

void SimulationIOHandler::reply(const Message& message)
{
  // post the reply, so it has some delay
  //! \todo better delay simulation? at least z21 message transfer time?
  m_kernel.ioContext().post(
    [this, data=copy(message)]()
    {
      static_cast<ClientKernel&>(m_kernel).receive(*reinterpret_cast<const Message*>(data.get()));
    });
}

void SimulationIOHandler::replyLanSystemStateDataChanged()
{
  LanSystemStateDataChanged message;

  if(m_emergencyStop)
    message.centralState |= Z21_CENTRALSTATE_EMERGENCYSTOP;
  if(!m_trackPowerOn)
    message.centralState |= Z21_CENTRALSTATE_TRACKVOLTAGEOFF;

  reply(message);
}

}
