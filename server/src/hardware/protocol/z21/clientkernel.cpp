/**
 * server/src/hardware/protocol/z21/clientkernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2024 Reinder Feenstra
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
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../protocol/dcc/dcc.hpp"
#include "../../input/inputcontroller.hpp"
#include "../../output/outputcontroller.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../utils/inrange.hpp"

namespace Z21 {

ClientKernel::ClientKernel(std::string logId_, const ClientConfig& config, bool simulation)
  : Kernel(std::move(logId_))
  , m_simulation{simulation}
  , m_keepAliveTimer(m_ioContext)
  , m_inactiveDecoderPurgeTimer(m_ioContext)
  , m_schedulePendingRequestTimer(m_ioContext)
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

  auto matchedRequest = matchPendingReplyAndRemove(message);

  switch(message.header())
  {
    case LAN_X:
    {
      const auto& lanX = static_cast<const LanX&>(message);

      if(!LanX::isChecksumValid(lanX))
        break;

      switch(lanX.xheader)
      {
        case LAN_X_TURNOUT_INFO:
          if(message.dataLen() == sizeof(LanXTurnoutInfo))
          {
            const auto& reply = static_cast<const LanXTurnoutInfo&>(message);
            OutputPairValue value = OutputPairValue::Undefined;
            if(!reply.positionUnknown())
            {
              value = reply.state() ? OutputPairValue::Second : OutputPairValue::First;
            }

            EventLoop::call(
              [this, address=reply.address(), value]()
              {
                m_outputController->updateOutputValue(OutputChannel::Accessory, address, value);
              });
          }
          break;

        case LAN_X_EXT_ACCESSORY_INFO:
          if(message.dataLen() == sizeof(LanXExtAccessoryInfo))
          {
            const auto& reply = static_cast<const LanXExtAccessoryInfo&>(message);
            if(reply.isDataValid())
            {
              EventLoop::call(
                [this, address=reply.address(), value=reply.aspect()]()
                {
                  m_outputController->updateOutputValue(OutputChannel::DCCext, address, value);
                });
            }
          }
          break;

        case LAN_X_BC:
          if(message == LanXBCTrackPowerOff() || message == LanXBCTrackShortCircuit())
          {
            EventLoop::call(
              [this]()
              {
                if(m_trackPowerOn != TriState::False)
                {
                  m_trackPowerOn = TriState::False;
                  m_emergencyStop = TriState::False;
                  if(m_onTrackPowerChanged)
                    m_onTrackPowerChanged(false, false);
                }
              });
          }
          else if(message == LanXBCTrackPowerOn())
          {
            EventLoop::call(
              [this]()
              {
                if(m_trackPowerOn != TriState::True || m_emergencyStop != TriState::False)
                {
                  m_trackPowerOn = TriState::True;
                  m_emergencyStop = TriState::False;
                  if(m_onTrackPowerChanged)
                    m_onTrackPowerChanged(true, false);
                }
              });
          }
          break;

        case LAN_X_BC_STOPPED:
          if(message == LanXBCStopped())
          {
            EventLoop::call(
              [this]()
              {
                if(m_emergencyStop != TriState::True)
                {
                  m_emergencyStop = TriState::True;
                  m_trackPowerOn = TriState::True;

                  if(m_onTrackPowerChanged)
                    m_onTrackPowerChanged(true, true);
                }
              });
          }
          break;

        case LAN_X_LOCO_INFO:
        {
          bool isAnswerToOurRequest = false;

          if(matchedRequest)
          {
            auto msgData = matchedRequest.value().messageBytes.data();
            const LanX& requestMsg = *reinterpret_cast<const LanX *>(msgData);

            // If we explicitly requested loco info then we treat it as external change
            if(requestMsg.xheader != LAN_X_GET_LOCO_INFO)
              isAnswerToOurRequest = true;
          }

          if(message.dataLen() >= LanXLocoInfo::minMessageSize && message.dataLen() <= LanXLocoInfo::maxMessageSize)
          {
            const auto& reply = static_cast<const LanXLocoInfo&>(message);

            //NOTE: there is also a function at index 0, hence +1
            const int functionIndexMax = std::min(reply.functionIndexMax(), LanXLocoInfo::supportedFunctionIndexMax);
            bool val[LanXLocoInfo::supportedFunctionIndexMax + 1] = {};

            for(int i = 0; i <= functionIndexMax; i++)
            {
              val[i] = reply.getFunction(i);
            }

            LocoCache &cache = getLocoCache(reply.address());

            DecoderChangeFlags changes = DecoderChangeFlags(0);

            //Rescale everything to 126 steps
            int currentSpeedStep = reply.speedStep();
            if(reply.speedSteps() != 126)
            {
              currentSpeedStep = float(currentSpeedStep) / float(reply.speedSteps()) * 126.0;
              if(abs(currentSpeedStep - cache.lastReceivedSpeedStep) < 5)
                currentSpeedStep = cache.lastReceivedSpeedStep; //Consider it a rounding error
            }

            // For answers to our own requests we don't care about direction and speed step
            if(cache.lastReceivedSpeedStep != currentSpeedStep)
            {
                if(!isAnswerToOurRequest)
                  changes |= DecoderChangeFlags::SpeedSteps;
                cache.lastReceivedSpeedStep = currentSpeedStep;
            }

            if(reply.speedSteps() != cache.speedSteps)
            {
              if(!isAnswerToOurRequest)
                changes |= DecoderChangeFlags::SpeedSteps;
              cache.speedSteps = reply.speedSteps();
            }

            //Emergency stop is urgent so bypass timeout
            //Direction is not propagated back so it shouldn't start looping
            //We bypass timeout also for Direction change.
            //It can at worst cause a short flickering changing direction n times and then settle down
            if(reply.direction() != cache.direction)
            {
              if(!isAnswerToOurRequest)
                changes |= DecoderChangeFlags::Direction;
              cache.direction = reply.direction();
            }

            if(reply.isEmergencyStop() || reply.isEmergencyStop() != cache.isEStop)
            {
              //Force change when emergency stop is set to be sure it gets received
              //as soon as possible
              changes |= DecoderChangeFlags::EmergencyStop;
              cache.isEStop = reply.isEmergencyStop();
            }

            //Do not update last seen time to avoid ignoring genuine user commands
            //Store last received speed step converted to 126 steps scale
            cache.lastReceivedSpeedStep = currentSpeedStep;

            // Update last seen time to prevent decoder to be purged
            cache.lastSetTime = std::chrono::steady_clock::now();

            if(isAnswerToOurRequest && changes == DecoderChangeFlags(0))
            {
              // No need to notify Decoder
              break;
            }

            EventLoop::call(
              [this, address=reply.address(), isEStop=reply.isEmergencyStop(),
              speed = reply.speedStep(), speedMax=reply.speedSteps(),
              dir = reply.direction(), val, functionIndexMax, changes]()
              {
                try
                {
                  if(auto decoder = m_decoderController->getDecoder(DCC::getProtocol(address), address))
                  {
                    float throttle = Decoder::speedStepToThrottle(speed, speedMax);

                    if(has(changes, DecoderChangeFlags::EmergencyStop))
                    {
                      m_isUpdatingDecoderFromKernel = true;
                      decoder->emergencyStop = isEStop;
                    }

                    if(has(changes, DecoderChangeFlags::Direction))
                    {
                      m_isUpdatingDecoderFromKernel = true;
                      decoder->direction = dir;
                    }

                    if(has(changes, DecoderChangeFlags::Throttle | DecoderChangeFlags::SpeedSteps))
                    {
                      m_isUpdatingDecoderFromKernel = true;
                      decoder->throttle = throttle;
                    }

                    //Reset flag guard at end
                    m_isUpdatingDecoderFromKernel = false;

                    //Function get always updated because we do not store a copy in cache
                    //so there is no way to tell in advance if they changed
                    for(int i = 0; i <= functionIndexMax; i++)
                    {
                      decoder->setFunctionValue(i, val[i]);
                    }
                  }
                }
                catch(...)
                {

                }
              });
          }
          break;
        }
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
        const TriState stopState = toTriState(isStop);

        EventLoop::call([this, trackPowerOn, stopState]()
          {
            if(m_trackPowerOn != trackPowerOn || m_emergencyStop != stopState)
            {
              m_trackPowerOn = trackPowerOn;
              m_emergencyStop = stopState;

              if(m_onTrackPowerChanged)
                m_onTrackPowerChanged(trackPowerOn == TriState::True,
                                      stopState == TriState::True);
            }
          });
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
  assert(isEventLoopThread());

  if(m_trackPowerOn != TriState::True || m_emergencyStop != TriState::False)
  {
    m_ioContext.post(
      [this]()
      {
        send(LanXSetTrackPowerOn());
      });
  }
}

void ClientKernel::trackPowerOff()
{
  assert(isEventLoopThread());

  if(m_trackPowerOn != TriState::False || m_emergencyStop != TriState::False)
  {
    m_ioContext.post(
      [this]()
      {
        send(LanXSetTrackPowerOff());
      });
  }
}

void ClientKernel::emergencyStop()
{
  assert(isEventLoopThread());

  if(m_trackPowerOn != TriState::True || m_emergencyStop != TriState::True)
  {
    m_ioContext.post(
      [this]()
      {
        send(LanXSetStop());
      });
  }
}

void ClientKernel::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  const uint16_t address = decoder.address;
  const bool longAddress = decoder.protocol == DecoderProtocol::DCCLong;

  const Direction direction = decoder.direction;
  const float throttle = decoder.throttle;
  const int speedSteps = decoder.speedSteps;
  const bool isEStop = decoder.emergencyStop;

  TriState funcVal = TriState::Undefined;
  if(const auto& f = decoder.getFunction(functionNumber))
    funcVal = toTriState(f->value);

  if(m_isUpdatingDecoderFromKernel)
  {
    //This change was caused by Z21 message so there is not point
    //on informing back Z21 with another message
    //Skip updating LocoCache again which might already be
    //at a new value (EventLoop is slower to process callbacks)
    //But reset the guard to allow Train and other parts of code
    //to react to this change and further edit decoder state
    m_isUpdatingDecoderFromKernel = false;
    return;
  }

  m_ioContext.post([this, address, longAddress, direction, throttle, speedSteps, isEStop, changes, functionNumber, funcVal]()
    {
      LanXSetLocoDrive cmd;
      cmd.setAddress(address, longAddress);

      cmd.setSpeedSteps(speedSteps);
      int speedStep = Decoder::throttleToSpeedStep(throttle, cmd.speedSteps());

      // Decoder max speed steps must be set for the message to be correctly
      // distinguished from LAN_X_SET_LOCO_FUNCTION
      cmd.setSpeedStep(speedStep);
      cmd.setDirection(direction);

      LocoCache &cache = getLocoCache(address);

      bool changed = false;
      if(has(changes, DecoderChangeFlags::Direction) && cache.direction != direction)
      {
        changed = true;
      }

      if(has(changes, DecoderChangeFlags::Throttle | DecoderChangeFlags::SpeedSteps | DecoderChangeFlags::EmergencyStop))
      {
        if(has(changes, DecoderChangeFlags::EmergencyStop) && isEStop != cache.isEStop)
        {
          if(isEStop)
            cmd.setEmergencyStop();
          changed = true;
        }

        if(!isEStop && (speedSteps != cache.speedSteps || speedStep != cache.speedStep))
        {
          changed = true;
        }
      }

      if(has(changes, DecoderChangeFlags::FunctionValue))
      {
        //This is independent of LanXSetLocoDrive
        if(functionNumber <= LanXSetLocoFunction::functionNumberMax && funcVal != TriState::Undefined)
        {
          send(LanXSetLocoFunction(
            address, longAddress,
            static_cast<uint8_t>(functionNumber),
            funcVal == TriState::True ? LanXSetLocoFunction::SwitchType::On : LanXSetLocoFunction::SwitchType::Off));
        }
      }

      if(changed)
      {
        cache.speedSteps = cmd.speedSteps();
        cache.speedStep = cmd.speedStep();
        cache.direction = cmd.direction();
        cache.isEStop = cmd.isEmergencyStop();

        // Update last seen time to prevent decoder to be purged
        cache.lastSetTime = std::chrono::steady_clock::now();
      }

      if(changed)
      {
        cmd.updateChecksum();
        send(cmd);
      }
    });
}

bool ClientKernel::setOutput(OutputChannel channel, uint16_t address, OutputValue value)
{
  assert(inRange<uint32_t>(address, outputAddressMin, outputAddressMax));

  if(channel == OutputChannel::Accessory)
  {
    m_ioContext.post(
      [this, address, port=std::get<OutputPairValue>(value) == OutputPairValue::Second]()
      {
        send(LanXSetTurnout(address, port, true));
        // TODO: sent deactivate after switch time, at least 50ms, see documentation
        // TODO: add some kind of queue if queing isn't supported?? requires at least v1.24 (DR5000 v1.5.5 has v1.29)
      });
    return true;
  }
  else if(channel == OutputChannel::DCCext)
  {
    if(m_firmwareVersionMajor == 1 && m_firmwareVersionMinor < 40)
    {
      Log::log(logId, LogMessage::W2020_DCCEXT_RCN213_IS_NOT_SUPPORTED);
      return false;
    }

    if(inRange<int16_t>(std::get<int16_t>(value), std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max())) /*[[likely]]*/
    {
      m_ioContext.post(
        [this, address, data=static_cast<uint8_t>(std::get<int16_t>(value))]()
        {
          send(LanXSetExtAccessory(address, data));
        });
      return true;
    }
  }
  return false;
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
  startInactiveDecoderPurgeTimer();
}

void ClientKernel::onStop()
{
  send(LanLogoff());

  m_keepAliveTimer.cancel();
  m_inactiveDecoderPurgeTimer.cancel();
  m_schedulePendingRequestTimer.cancel();
  m_locoCache.clear();
  m_pendingRequests.clear();
}

void ClientKernel::send(const Message& message, bool wantReply, uint8_t customRetryCount)
{
  if(m_ioHandler->send(message))
  {
    if(m_config.debugLogRXTX)
      EventLoop::call(
        [this, msg=toString(message)]()
        {
          Log::log(logId, LogMessage::D2001_TX_X, msg);
        });

    if(wantReply)
    {
      PendingRequest request;
      request.reply = getReplyType(message);

      if(request.reply.header != MessageReplyType::noReply)
      {
        if(customRetryCount > 0)
        {
          request.retryCount = customRetryCount;
        }
        else
        {
          // Calculate from priority
          switch (request.reply.priority())
          {
            case MessageReplyType::Priority::Low:
              request.retryCount = 1;
              break;

            default:
            case MessageReplyType::Priority::Normal:
              request.retryCount = 2;
              break;

            case MessageReplyType::Priority::Urgent:
              request.retryCount = 5;
              break;
          }
        }

        // Save copy of original message
        request.messageBytes.resize(message.dataLen());
        std::memcpy(request.messageBytes.data(), &message, message.dataLen());

        // Enque pending request
        addPendingRequest(request);
      }
    }
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

void ClientKernel::startInactiveDecoderPurgeTimer()
{
  assert(ClientConfig::purgeInactiveDecoderInternal > 0);
  m_inactiveDecoderPurgeTimer.expires_after(boost::asio::chrono::seconds(ClientConfig::purgeInactiveDecoderInternal));
  m_inactiveDecoderPurgeTimer.async_wait(std::bind(&ClientKernel::inactiveDecoderPurgeTimerExpired, this, std::placeholders::_1));
}

void ClientKernel::inactiveDecoderPurgeTimerExpired(const boost::system::error_code& ec)
{
  if(ec)
    return;

  const auto purgeTime = std::chrono::steady_clock::now() - std::chrono::seconds(ClientConfig::purgeInactiveDecoderInternal);

  auto it = m_locoCache.begin();
  while(it != m_locoCache.end())
  {
    if(it->second.lastSetTime < purgeTime)
    {
      it = m_locoCache.erase(it);
    }
    else
    {
      it++;
    }
  }

  startInactiveDecoderPurgeTimer();
}

ClientKernel::LocoCache& ClientKernel::getLocoCache(uint16_t dccAddr)
{
  auto it = m_locoCache.find(dccAddr);
  if(it == m_locoCache.end())
  {
    LocoCache item;
    item.dccAddress = dccAddr;
    it = m_locoCache.emplace(dccAddr, item).first;
  }

  return it->second;
}

void ClientKernel::addPendingRequest(const PendingRequest &request)
{
  //Enqueue this request to track reply from Z21
  bool wasEmpty = m_pendingRequests.empty();
  PendingRequest req = request;
  req.sendTime = std::chrono::steady_clock::now();
  m_pendingRequests.push_back(req);
  if(wasEmpty)
    startSchedulePendingRequestTimer();
}

std::optional<ClientKernel::PendingRequest> ClientKernel::matchPendingReplyAndRemove(const Message &message)
{
  const auto currentTime = std::chrono::steady_clock::now();

  //TODO: depends on priority and network estimated speed
  const auto timeout = std::chrono::seconds(3);

  for(auto request = m_pendingRequests.begin(); request != m_pendingRequests.end(); request++)
  {
    //If it's last retry and we exceeded timeout, skip request
    if(request->retryCount == 0 && (currentTime - request->sendTime) > timeout)
      continue;

    if(message.header() != request->reply.header)
      continue;

    if(message.header() == LAN_X)
    {
      const LanX& lanX = static_cast<const LanX&>(message);
      if(lanX.xheader != request->reply.xHeader)
        continue;

      if(request->reply.hasFlag(MessageReplyType::Flags::CheckDb0))
      {
        // Cast to any LanX message with a db0 to check its value
        const LanXGetStatus& hack = static_cast<const LanXGetStatus&>(lanX);
        if(hack.db0 != request->reply.db0)
          continue;
      }
    }

    if(request->reply.hasFlag(MessageReplyType::Flags::CheckAddress))
    {
      uint16_t address = 0;
      switch (message.header())
      {
        case LAN_GET_LOCO_MODE:
        {
          address = static_cast<const LanGetLocoMode&>(message).address();
          break;
        }

        case LAN_GET_TURNOUTMODE:
        {
          // NOTE: not (yet) supported
          break;
        }

        case LAN_X:
        {
          const LanX& lanX = static_cast<const LanX&>(message);
          switch (lanX.xheader)
          {
            case LAN_X_TURNOUT_INFO:
              address = static_cast<const LanXTurnoutInfo&>(lanX).address();
              break;

            case LAN_X_LOCO_INFO:
              address = static_cast<const LanXLocoInfo&>(lanX).address();
              break;

            default:
              break;
          }
        }

        default:
          break;
      }

      if(address != request->reply.address)
        continue;
    }

    if(request->reply.hasFlag(MessageReplyType::Flags::CheckSpeedStep))
    {
      if(message.header() == LAN_X
          && static_cast<const LanX&>(message).xheader == LAN_X_LOCO_INFO)
      {
        const LanXLocoInfo& locoInfo = static_cast<const LanXLocoInfo&>(message);
        if(locoInfo.speedAndDirection != request->reply.speedAndDirection)
          continue;
        if(locoInfo.speedSteps() != request->reply.speedSteps())
          continue;
      }
    }

    // We matched a previously sent request
    // NOTE: In theory we could have matched a reply generated by other clients operations
    // But for our purposes this should be fine

    // Remove it from pending queue
    PendingRequest copy = *request;
    m_pendingRequests.erase(request);
    return copy;
  }

  return {};
}

void ClientKernel::startSchedulePendingRequestTimer()
{
  //TODO: depends on priority and network estimated speed
  const auto timeout = std::chrono::seconds(1);

  m_schedulePendingRequestTimer.expires_after(timeout);
  m_schedulePendingRequestTimer.async_wait(std::bind(&ClientKernel::schedulePendingRequestTimerExpired, this, std::placeholders::_1));
}

void ClientKernel::schedulePendingRequestTimerExpired(const boost::system::error_code &ec)
{
  if(ec)
    return;

  rescheduleTimedoutRequests();
}

void ClientKernel::rescheduleTimedoutRequests()
{
  const auto deadlineTime = std::chrono::steady_clock::now();

  //TODO: depends on priority and network estimated speed
  const auto timeout = std::chrono::seconds(3);

  auto request = m_pendingRequests.begin();
  while(request != m_pendingRequests.end())
  {
    if((deadlineTime - request->sendTime) > timeout)
    {
      if(request->retryCount <= 0)
      {
        // Give up with this request and remove it
        request = m_pendingRequests.erase(request);
        continue;
      }

      // Decrement retry count
      request->retryCount--;

      // Re-schedule request
      auto msgData = request->messageBytes.data();
      const Message& requestMsg = *reinterpret_cast<const Message *>(msgData);

      // Send original request again but without adding it to pending queue
      send(requestMsg, false);

      // Restart timeout
      request->sendTime = std::chrono::steady_clock::now();
    }

    request++;
  }

  if(!m_pendingRequests.empty())
    startSchedulePendingRequestTimer();
}

}
