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
            response.calcChecksum();
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

        case LAN_X_SET_STOP:
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

        case 0xE3:
          if(const auto& getLocoInfo = static_cast<const LanXGetLocoInfo&>(message);
              getLocoInfo.db0 == 0xF0)
          {
            // not (yet) supported
          }
          break;

        case 0xE4:
          if(const auto& setLocoDrive = static_cast<const LanXSetLocoDrive&>(message);
              setLocoDrive.db0 >= 0x10 && setLocoDrive.db0 <= 0x13)
          {
            // not (yet) supported
          }
          else if(const auto& setLocoFunction = static_cast<const LanXSetLocoFunction&>(message);
                  setLocoFunction.db0 == 0xF8 &&
                  setLocoFunction.switchType() != LanXSetLocoFunction::SwitchType::Invalid)
          {
            // not (yet) supported
          }
          break;

        case 0xF1:
          if(message == LanXGetFirmwareVersion())
          {
            reply(LanXGetFirmwareVersionReply(firmwareVersionMajor, ServerConfig::firmwareVersionMinor));
          }
          break;
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
        reply(LanSetBroadcastFlags(m_broadcastFlags));
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
