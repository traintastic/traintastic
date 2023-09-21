/**
 * server/src/hardware/protocol/z21/clientkernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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

#include "clientkernel.hpp"
#include "messages.hpp"
#include "../xpressnet/messages.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../input/inputcontroller.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../utils/inrange.hpp"

namespace Z21 {

ClientKernel::ClientKernel(std::string logId_, const ClientConfig& config, bool simulation)
  : Kernel(std::move(logId_))
  , m_simulation{simulation}
  , m_keepAliveTimer(m_ioContext)
  , m_config{config}
{
}

void ClientKernel::setConfig(const ClientConfig& config)
{
  m_ioContext.post(
    [this, newConfig=config]()
    {
      m_config = newConfig;
    });
}

void ClientKernel::receive(const Message& message)
{
  if(m_config.debugLogRXTX)
    EventLoop::call(
      [this, msg=toString(message)]()
      {
        Log::log(logId, LogMessage::D2002_RX_X, msg);
      });

  switch(message.header())
  {
    case LAN_X:
    {
      const auto& lanX = static_cast<const LanX&>(message);

      if(!XpressNet::isChecksumValid(*reinterpret_cast<const XpressNet::Message*>(&lanX.xheader)))
        break;

      switch(lanX.xheader)
      {
        case LAN_X_BC:
          if(message == LanXBCTrackPowerOff() || message == LanXBCTrackShortCircuit())
          {
            if(m_trackPowerOn != TriState::False)
            {
              m_trackPowerOn = TriState::False;

              if(m_onTrackPowerOnChanged)
                EventLoop::call(
                  [this]()
                  {
                    m_onTrackPowerOnChanged(false);
                  });
            }
          }
          else if(message == LanXBCTrackPowerOn())
          {
            if(m_trackPowerOn != TriState::True)
            {
              m_trackPowerOn = TriState::True;

              if(m_onTrackPowerOnChanged)
                EventLoop::call(
                  [this]()
                  {
                    m_onTrackPowerOnChanged(true);
                  });
            }
          }
          break;

        case LAN_X_BC_STOPPED:
          if(message == LanXBCStopped())
          {
            if(m_emergencyStop != TriState::True)
            {
              m_emergencyStop = TriState::True;

              if(m_onEmergencyStop)
                EventLoop::call(
                  [this]()
                  {
                    m_onEmergencyStop();
                  });
            }
          }
          break;
      }
      break;
    }
    case LAN_GET_SERIAL_NUMBER:
      if(message.dataLen() == sizeof(LanGetSerialNumberReply))
      {
        const auto& reply = static_cast<const LanGetSerialNumberReply&>(message);

        if(m_serialNumber != reply.serialNumber())
        {
          m_serialNumber = reply.serialNumber();
          if(m_onSerialNumberChanged)
          {
            EventLoop::call(
              [this, serialNumber=m_serialNumber]()
              {
                m_onSerialNumberChanged(serialNumber);
              });
          }
        }
      }
      break;

    case LAN_GET_HWINFO:
      if(message.dataLen() == sizeof(LanGetHardwareInfoReply))
      {
        const auto& reply = static_cast<const LanGetHardwareInfoReply&>(message);

        if(m_hardwareType != reply.hardwareType() ||
            m_firmwareVersionMajor != reply.firmwareVersionMajor() ||
            m_firmwareVersionMinor != reply.firmwareVersionMinor())
        {
          m_hardwareType = reply.hardwareType();
          m_firmwareVersionMajor = reply.firmwareVersionMajor();
          m_firmwareVersionMinor = reply.firmwareVersionMinor();

          if(m_onHardwareInfoChanged)
          {
            EventLoop::call(
              [this, hardwareType=m_hardwareType, firmwareVersionMajor=m_firmwareVersionMajor, firmwareVersionMinor=m_firmwareVersionMinor]()
              {
                m_onHardwareInfoChanged(hardwareType, firmwareVersionMajor, firmwareVersionMinor);
              });
          }
        }
      }
      break;

    case LAN_RMBUS_DATACHANGED:
      if(m_inputController && message.dataLen() == sizeof(LanRMBusDataChanged))
      {
        const auto& data = static_cast<const LanRMBusDataChanged&>(message);

        for(uint8_t i = 0; i < LanRMBusDataChanged::feedbackStatusCount; i++)
        {
          const uint16_t index = data.groupIndex * LanRMBusDataChanged::feedbackStatusCount + i;
          const TriState value = toTriState(data.getFeedbackStatus(i));
          if(m_rbusFeedbackStatus[index] != value)
          {
            m_rbusFeedbackStatus[index] = value;

            EventLoop::call(
              [this, address=rbusAddressMin + index, value]()
              {
                m_inputController->updateInputValue(InputChannel::rbus, address, value);
              });
          }
        }
      }
      break;

    case LAN_LOCONET_DETECTOR:
      if(m_inputController && message.dataLen() >= sizeof(LanLocoNetDetector))
      {
        switch(static_cast<const LanLocoNetDetector&>(message).type)
        {
          case LanLocoNetDetector::Type::OccupancyDetector:
          {
            const auto& data = static_cast<const LanLocoNetDetectorOccupancyDetector&>(message);
            const uint16_t index = data.feedbackAddress();
            const TriState value = toTriState(data.isOccupied());
            if(m_loconetFeedbackStatus[index] != value)
            {
              m_loconetFeedbackStatus[index] = value;

              EventLoop::call(
                [this, address=loconetAddressMin + index, value]()
                {
                  m_inputController->updateInputValue(InputChannel::loconet, address, value);
                });
            }
            break;
          }
          case LanLocoNetDetector::Type::TransponderEntersBlock:
          case LanLocoNetDetector::Type::TransponderExitsBlock:
          case LanLocoNetDetector::Type::LissyLocoAddress:
          case LanLocoNetDetector::Type::LissyBlockStatus:
          case LanLocoNetDetector::Type::LissySpeed:
          case LanLocoNetDetector::Type::StationaryInterrogateRequest:
          case LanLocoNetDetector::Type::ReportAddress:
          case LanLocoNetDetector::Type::StatusRequestLissy:
            break; // not (yet) supported
        }
      }
      break;

    case LAN_GET_BROADCASTFLAGS:
      if(message.dataLen() == sizeof(LanGetBroadcastFlagsReply))
      {
        const auto& reply = static_cast<const LanGetBroadcastFlagsReply&>(message);
        m_broadcastFlags = reply.broadcastFlags();

        if(m_broadcastFlags != requiredBroadcastFlags)
        {
            Log::log(logId, LogMessage::W2019_Z21_BROADCAST_FLAG_MISMATCH);
        }
      }
      break;

    case LAN_SYSTEMSTATE_DATACHANGED:
    {
      if(message.dataLen() == sizeof(LanSystemStateDataChanged))
      {
        const auto& reply = static_cast<const LanSystemStateDataChanged&>(message);

        const bool isStop = reply.centralState & Z21_CENTRALSTATE_EMERGENCYSTOP;
        const bool isTrackPowerOn = (reply.centralState & Z21_CENTRALSTATE_TRACKVOLTAGEOFF) == 0
                                    && (reply.centralState & Z21_CENTRALSTATE_SHORTCIRCUIT) == 0;

        const TriState trackPowerOn = toTriState(isTrackPowerOn);
        if(m_trackPowerOn != trackPowerOn)
        {
          m_trackPowerOn = trackPowerOn;

          if(m_onTrackPowerOnChanged)
            EventLoop::call(
              [this, isTrackPowerOn]()
              {
                m_onTrackPowerOnChanged(isTrackPowerOn);
              });
        }

        if(m_emergencyStop != TriState::True && isStop)
        {
          m_emergencyStop = TriState::True;

          if(m_onEmergencyStop)
            EventLoop::call(
              [this]()
              {
                m_onEmergencyStop();
              });
        }
      }
      break;
    }

    case LAN_GET_CODE:
    case LAN_LOGOFF:
    case LAN_SET_BROADCASTFLAGS:
    case LAN_GET_LOCO_MODE:
    case LAN_SET_LOCO_MODE:
    case LAN_GET_TURNOUTMODE:
    case LAN_SET_TURNOUTMODE:
    case LAN_RMBUS_GETDATA:
    case LAN_RMBUS_PROGRAMMODULE:
    case LAN_SYSTEMSTATE_GETDATA:
    case LAN_RAILCOM_DATACHANGED:
    case LAN_RAILCOM_GETDATA:
    case LAN_LOCONET_Z21_RX:
    case LAN_LOCONET_Z21_TX:
    case LAN_LOCONET_FROM_LAN:
    case LAN_LOCONET_DISPATCH_ADDR:
    case LAN_CAN_DETECTOR:
      break; // not (yet) supported
  }
}

void ClientKernel::trackPowerOn()
{
  m_ioContext.post(
    [this]()
    {
      if(m_trackPowerOn != TriState::True || m_emergencyStop != TriState::False)
        send(LanXSetTrackPowerOn());
    });
}

void ClientKernel::trackPowerOff()
{
  m_ioContext.post(
    [this]()
    {
      if(m_trackPowerOn != TriState::False)
        send(LanXSetTrackPowerOff());
    });
}

void ClientKernel::emergencyStop()
{
  m_ioContext.post(
    [this]()
    {
      if(m_emergencyStop != TriState::True)
        send(LanXSetStop());
    });
}

void ClientKernel::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Direction | DecoderChangeFlags::Throttle | DecoderChangeFlags::SpeedSteps))
  {
    LanXSetLocoDrive cmd;
    cmd.setAddress(decoder.address, decoder.protocol == DecoderProtocol::DCCLong);

    switch(decoder.speedSteps)
    {
      case 14:
      {
        const uint8_t speedStep = Decoder::throttleToSpeedStep<uint8_t>(decoder.throttle, 14);
        cmd.db0 = 0x10;
        if(decoder.emergencyStop)
          cmd.speedAndDirection = 0x01;
        else if(speedStep > 0)
          cmd.speedAndDirection = speedStep + 1;
        break;
      }
      case 28:
      {
        uint8_t speedStep = Decoder::throttleToSpeedStep<uint8_t>(decoder.throttle, 28);
        cmd.db0 = 0x12;
        if(decoder.emergencyStop)
          cmd.speedAndDirection = 0x01;
        else if(speedStep > 0)
        {
          speedStep++;
          cmd.speedAndDirection = ((speedStep & 0x01) << 4) | (speedStep >> 1);
        }
        break;
      }
      case 126:
      case 128:
      default:
      {
        const uint8_t speedStep = Decoder::throttleToSpeedStep<uint8_t>(decoder.throttle, 126);
        cmd.db0 = 0x13;
        if(decoder.emergencyStop)
          cmd.speedAndDirection = 0x01;
        else if(speedStep > 0)
          cmd.speedAndDirection = speedStep + 1;
        break;
      }
    }

    assert(decoder.direction.value() != Direction::Unknown);
    if(decoder.direction.value() == Direction::Forward)
      cmd.speedAndDirection |= 0x80;

    cmd.checksum = XpressNet::calcChecksum(*reinterpret_cast<const XpressNet::Message*>(&cmd.xheader));
    postSend(cmd);
  }
  else if(has(changes, DecoderChangeFlags::FunctionValue))
  {
    if(functionNumber <= LanXSetLocoFunction::functionNumberMax)
      if(const auto& f = decoder.getFunction(functionNumber))
        postSend(LanXSetLocoFunction(
          decoder.address, decoder.protocol == DecoderProtocol::DCCLong,
          static_cast<uint8_t>(functionNumber),
          f->value ? LanXSetLocoFunction::SwitchType::On : LanXSetLocoFunction::SwitchType::Off));
  }
}

bool ClientKernel::setOutput(uint16_t address, bool value)
{
  assert(inRange<uint32_t>(address, outputAddressMin, outputAddressMax));

  m_ioContext.post(
    [this, address, value]()
    {
      send(LanXSetTurnout(address, value, true));
    });

  return true;
}

void ClientKernel::simulateInputChange(uint32_t channel, uint32_t address, SimulateInputAction action)
{
  if(!m_simulation)
    return;

  m_ioContext.post(
    [this, channel, address, action]()
    {
      (void)address;
      switch(channel)
      {
        case InputChannel::rbus:
        {
          if((action == SimulateInputAction::SetFalse && m_rbusFeedbackStatus[address - rbusAddressMin] == TriState::False) ||
              (action == SimulateInputAction::SetTrue && m_rbusFeedbackStatus[address - rbusAddressMin] == TriState::True))
            return; // no change

          LanRMBusDataChanged message;
          message.groupIndex = (address - rbusAddressMin) / LanRMBusDataChanged::feedbackStatusCount;

          for(uint8_t i = 0; i < LanRMBusDataChanged::feedbackStatusCount; i++)
          {
            const uint32_t n = static_cast<uint32_t>(message.groupIndex) * LanRMBusDataChanged::feedbackStatusCount + i;
            if(address == rbusAddressMin + n)
            {
              switch(action)
              {
                case SimulateInputAction::SetFalse:
                  message.setFeedbackStatus(i, false);
                  break;

                case SimulateInputAction::SetTrue:
                  message.setFeedbackStatus(i, true);
                  break;

                case SimulateInputAction::Toggle:
                  message.setFeedbackStatus(i, m_rbusFeedbackStatus[n] != TriState::True);
                  break;
              }
            }
            else
              message.setFeedbackStatus(i, m_rbusFeedbackStatus[n] == TriState::True);
          }

          receive(message);

          break;
        }
        case InputChannel::loconet:
        {
          const uint16_t feedbackAddress = address - loconetAddressMin;
          bool occupied;
          switch(action)
          {
            case SimulateInputAction::SetFalse:
              if(m_loconetFeedbackStatus[feedbackAddress] == TriState::False)
                return; // no change
              occupied = false;
              break;

            case SimulateInputAction::SetTrue:
              if(m_loconetFeedbackStatus[feedbackAddress] == TriState::True)
                return; // no change
              occupied = true;
              break;

            case SimulateInputAction::Toggle:
              occupied = m_loconetFeedbackStatus[feedbackAddress] != TriState::True;
              break;

            default:
              assert(false);
              return;
          }
          receive(LanLocoNetDetectorOccupancyDetector(feedbackAddress, occupied));
          break;
        }
      }
    });
}

void ClientKernel::onStart()
{
  // reset all state values
  m_broadcastFlags = BroadcastFlags::None;
  m_broadcastFlagsRetryCount = 0;
  m_serialNumber = 0;
  m_hardwareType = HWT_UNKNOWN;
  m_firmwareVersionMajor = 0;
  m_firmwareVersionMinor = 0;
  m_trackPowerOn = TriState::Undefined;
  m_emergencyStop = TriState::Undefined;
  m_rbusFeedbackStatus.fill(TriState::Undefined);
  m_loconetFeedbackStatus.fill(TriState::Undefined);

  send(LanGetSerialNumber());
  send(LanGetHardwareInfo());

  send(LanSetBroadcastFlags(requiredBroadcastFlags));

  send(LanGetBroadcastFlags());

  send(LanSystemStateGetData());

  startKeepAliveTimer();
}

void ClientKernel::onStop()
{
  send(LanLogoff());
}

void ClientKernel::send(const Message& message)
{
  if(m_ioHandler->send(message))
  {
    if(m_config.debugLogRXTX)
      EventLoop::call(
        [this, msg=toString(message)]()
        {
          Log::log(logId, LogMessage::D2001_TX_X, msg);
        });
  }
  else
  {} // log message and go to error state
}

void ClientKernel::startKeepAliveTimer()
{
  if(m_broadcastFlags == BroadcastFlags::None && m_broadcastFlagsRetryCount == maxBroadcastFlagsRetryCount)
  {
    Log::log(logId, LogMessage::W2019_Z21_BROADCAST_FLAG_MISMATCH);
    m_broadcastFlagsRetryCount++; //Log only once
  }

  if(m_broadcastFlags == BroadcastFlags::None && m_broadcastFlagsRetryCount < maxBroadcastFlagsRetryCount)
  {
    //Request BC flags as keep alive message
    m_keepAliveTimer.expires_after(boost::asio::chrono::seconds(2));
  }
  else
  {
    //Normal keep alive
    assert(ClientConfig::keepAliveInterval > 0);
    m_keepAliveTimer.expires_after(boost::asio::chrono::seconds(ClientConfig::keepAliveInterval));
  }

  m_keepAliveTimer.async_wait(std::bind(&ClientKernel::keepAliveTimerExpired, this, std::placeholders::_1));
}

void ClientKernel::keepAliveTimerExpired(const boost::system::error_code& ec)
{
  if(ec)
    return;

  if(m_broadcastFlags == BroadcastFlags::None)
  {
    //Request BC flags as keep alive message
    m_broadcastFlagsRetryCount++;
    send(LanSetBroadcastFlags(requiredBroadcastFlags));
    send(LanGetBroadcastFlags());
  }
  else
  {
    //Normal keep alive
    send(LanSystemStateGetData());
  }

  startKeepAliveTimer();
}

}
