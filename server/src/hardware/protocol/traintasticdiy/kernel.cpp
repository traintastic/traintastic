/**
 * server/src/hardware/protocol/traintasticdiy/kernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#include "kernel.hpp"
#include "messages.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../decoder/list/decoderlist.hpp"
#include "../../input/inputcontroller.hpp"
#include "../../output/outputcontroller.hpp"
#include "../../../utils/inrange.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../log/log.hpp"
#include "../../../world/world.hpp"

namespace TraintasticDIY {

constexpr TriState toTriState(InputState value)
{
  switch(value)
  {
    case InputState::False:
      return TriState::False;

    case InputState::True:
      return TriState::True;

    case InputState::Undefined:
    case InputState::Invalid:
      break;
  }
  return TriState::Undefined;
}

constexpr TriState toTriState(OutputState value)
{
  switch(value)
  {
    case OutputState::False:
      return TriState::False;

    case OutputState::True:
      return TriState::True;

    case OutputState::Undefined:
    case OutputState::Invalid:
      break;
  }
  return TriState::Undefined;
}

Kernel::Kernel(World& world, const Config& config, bool simulation)
  : m_world{world}
  , m_ioContext{1}
  , m_simulation{simulation}
  , m_heartbeatTimeout{m_ioContext}
  , m_inputController{nullptr}
  , m_outputController{nullptr}
  , m_config{config}
#ifndef NDEBUG
  , m_started{false}
#endif
{
}

void Kernel::setConfig(const Config& config)
{
  m_ioContext.post(
    [this, newConfig=config]()
    {
      m_config = newConfig;
    });
}

void Kernel::start()
{
  assert(m_ioHandler);
  assert(!m_started);
  assert(m_inputValues.empty());
  assert(m_outputValues.empty());
  assert(m_throttleSubscriptions.empty());
  assert(m_decoderSubscriptions.empty());

  m_featureFlagsSet = false;
  m_featureFlags1 = FeatureFlags1::None;
  m_featureFlags2 = FeatureFlags2::None;
  m_featureFlags3 = FeatureFlags3::None;
  m_featureFlags4 = FeatureFlags4::None;

  m_thread = std::thread(
    [this]()
    {
      setThreadName("traintasticdiy");
      auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
      m_ioContext.run();
    });

  m_ioContext.post(
    [this]()
    {
      m_ioHandler->start();

      send(GetInfo());
      send(GetFeatures());

      restartHeartbeatTimeout();

      if(m_onStarted)
        EventLoop::call(
          [this]()
          {
            m_onStarted();
          });
    });

#ifndef NDEBUG
  m_started = true;
#endif
}

void Kernel::stop()
{
  for(auto& it : m_decoderSubscriptions)
    it.second.connection.disconnect();

  m_ioContext.post(
    [this]()
    {
      m_heartbeatTimeout.cancel();
      m_ioHandler->stop();
    });

  m_ioContext.stop();

  m_thread.join();

  m_inputValues.clear();
  m_outputValues.clear();
  m_throttleSubscriptions.clear();
  m_decoderSubscriptions.clear();

#ifndef NDEBUG
  m_started = false;
#endif
}

void Kernel::receive(const Message& message)
{
  if(m_config.debugLogRXTX && (message != Heartbeat() || m_config.debugLogHeartbeat))
    EventLoop::call(
      [this, msg=toString(message)]()
      {
        Log::log(m_logId, LogMessage::D2002_RX_X, msg);
      });

  restartHeartbeatTimeout();

  switch(message.opCode)
  {
    case OpCode::Heartbeat:
      break;

    case OpCode::SetInputState:
    {
      if(!m_featureFlagsSet || !hasFeatureInput())
        break;

      const auto& setInputState = static_cast<const SetInputState&>(message);
      const uint16_t address = setInputState.address();
      if(inRange(address, ioAddressMin, ioAddressMax))
      {
        auto it = m_inputValues.find(address);
        if(it == m_inputValues.end() || it->second != setInputState.state)
        {
          m_inputValues[address] = setInputState.state;

          EventLoop::call(
            [this, address, state=setInputState.state]()
            {
              if(state == InputState::Invalid)
              {
                if(m_inputController->inputMap().count({InputController::defaultInputChannel, address}) != 0)
                  Log::log(m_logId, LogMessage::W2004_INPUT_ADDRESS_X_IS_INVALID, address);
              }
              else
                m_inputController->updateInputValue(InputController::defaultInputChannel, address, toTriState(state));
            });
        }
      }
      break;
    }
    case OpCode::SetOutputState:
    {
      if(!m_featureFlagsSet || !hasFeatureOutput())
        break;

      const auto& setOutputState = static_cast<const SetOutputState&>(message);
      const uint16_t address = setOutputState.address();
      if(inRange(address, ioAddressMin, ioAddressMax))
      {
        auto it = m_outputValues.find(address);
        if(it == m_outputValues.end() || it->second != setOutputState.state)
        {
          m_outputValues[address] = setOutputState.state;

          EventLoop::call(
            [this, address, state=setOutputState.state]()
            {
              if(state == OutputState::Invalid)
              {
                if(m_outputController->outputMap().count({OutputController::defaultOutputChannel, address}) != 0)
                  Log::log(m_logId, LogMessage::W2005_OUTPUT_ADDRESS_X_IS_INVALID, address);
              }
              else
                m_outputController->updateOutputValue(OutputController::defaultOutputChannel, address, toTriState(state));
            });
        }
      }
      break;
    }
    case OpCode::ThrottleSubUnsub:
    {
      if(!m_featureFlagsSet || !hasFeatureThrottle())
        break;

      const auto& subUnsub = static_cast<const ThrottleSubUnsub&>(message);
      switch(subUnsub.action())
      {
        case ThrottleSubUnsub::Unsubscribe:
          throttleUnsubscribe(subUnsub.throttleId(), {subUnsub.address(), subUnsub.isLongAddress()});
          send(subUnsub);
          break;

        case ThrottleSubUnsub::Subscribe:
          throttleSubscribe(subUnsub.throttleId(), {subUnsub.address(), subUnsub.isLongAddress()});
          EventLoop::call(
            [this, subUnsub]()
            {
              if(auto decoder = getDecoder(subUnsub.address(), subUnsub.isLongAddress()))
              {
                uint8_t speedMax = 0;
                uint8_t speed = 0;

                if(!decoder->emergencyStop)
                {
                  speedMax = decoder->speedSteps.value();
                  if(speedMax == Decoder::speedStepsAuto)
                    speedMax = std::numeric_limits<uint8_t>::max();
                  speed = Decoder::throttleToSpeedStep(decoder->throttle, speedMax);
                }

                postSend(ThrottleSetSpeedDirection(subUnsub.throttleId(), subUnsub.address(), subUnsub.isLongAddress(), speed, speedMax, decoder->direction));
                for(const auto& function : *decoder->functions)
                  postSend(ThrottleSetFunction(subUnsub.throttleId(), subUnsub.address(), subUnsub.isLongAddress(), function->number, function->value));
              }
            });
          break;
      }
      break;
    }
    case OpCode::ThrottleSetFunction:
    {
      if(!m_featureFlagsSet || !hasFeatureThrottle())
        break;

      const auto& throttleSetFunction = static_cast<const ThrottleSetFunction&>(message);

      throttleSubscribe(throttleSetFunction.throttleId(), {throttleSetFunction.address(), throttleSetFunction.isLongAddress()});

      EventLoop::call(
        [this, throttleSetFunction]()
        {
          if(auto decoder = getDecoder(throttleSetFunction.address(), throttleSetFunction.isLongAddress()))
          {
            bool value = false;

            if(auto function = decoder->getFunction(throttleSetFunction.functionNumber()))
            {
              function->value = throttleSetFunction.functionValue();
              if(function->value != throttleSetFunction.functionValue())
              {
                send(ThrottleSetFunction(
                  throttleSetFunction.throttleId(),
                  throttleSetFunction.address(),
                  throttleSetFunction.isLongAddress(),
                  throttleSetFunction.functionNumber(),
                  function->value));
              }
            }
            else
            {
              // warning or debug?
              send(ThrottleSetFunction(
                throttleSetFunction.throttleId(),
                throttleSetFunction.address(),
                throttleSetFunction.isLongAddress(),
                throttleSetFunction.functionNumber(),
                value));
            }
          }
        });
      break;
    }
    case OpCode::ThrottleSetSpeedDirection:
    {
      if(!m_featureFlagsSet || !hasFeatureThrottle())
        break;

      const auto& throttleSetSpeedDirection = static_cast<const ThrottleSetSpeedDirection&>(message);

      throttleSubscribe(throttleSetSpeedDirection.throttleId(), {throttleSetSpeedDirection.address(), throttleSetSpeedDirection.isLongAddress()});

      EventLoop::call(
        [this, throttleSetSpeedDirection]()
        {
          if(auto decoder = getDecoder(throttleSetSpeedDirection.address(), throttleSetSpeedDirection.isLongAddress()))
          {
            if(throttleSetSpeedDirection.isSpeedSet())
            {
              decoder->emergencyStop = throttleSetSpeedDirection.isEmergencyStop();
              if(!throttleSetSpeedDirection.isEmergencyStop())
                decoder->throttle = throttleSetSpeedDirection.throttle();
            }
            if(throttleSetSpeedDirection.isDirectionSet())
              decoder->direction = throttleSetSpeedDirection.direction();
          }
        });
      break;
    }
    case OpCode::Features:
    {
      const auto& features = static_cast<const Features&>(message);
      m_featureFlagsSet = true;
      m_featureFlags1 = features.featureFlags1;
      m_featureFlags2 = features.featureFlags2;
      m_featureFlags3 = features.featureFlags3;
      m_featureFlags4 = features.featureFlags4;

      if(hasFeatureInput())
        EventLoop::call(
          [this]()
          {
            for(const auto& it : m_inputController->inputMap())
              postSend(GetInputState(static_cast<uint16_t>(it.first.address)));
          });

      if(hasFeatureOutput())
        EventLoop::call(
          [this]()
          {
            for(const auto& it : m_outputController->outputMap())
              postSend(GetOutputState(static_cast<uint16_t>(it.first.address)));
          });
      break;
    }
    case OpCode::Info:
    {
      const auto& info = static_cast<const InfoBase&>(message);
      EventLoop::call(
        [this, text=std::string(info.text())]()
        {
          Log::log(m_logId, LogMessage::I2005_X, text);
        });
      break;
    }
    case OpCode::GetInfo:
    case OpCode::GetFeatures:
    case OpCode::GetOutputState:
    case OpCode::GetInputState:
      assert(false);
      break;
  }
}

bool Kernel::setOutput(uint16_t address, bool value)
{
  postSend(SetOutputState(address, value ? OutputState::True : OutputState::False));
  return true;
}

void Kernel::simulateInputChange(uint16_t address, SimulateInputAction action)
{
  if(m_simulation)
    m_ioContext.post(
      [this, address, action]()
      {
        TraintasticDIY::InputState state;
        auto it = m_inputValues.find(address);
        switch(action)
        {
          case SimulateInputAction::SetFalse:
            if(it != m_inputValues.end() && it->second == InputState::False)
              return; // no change
            state = InputState::False;
            break;

          case SimulateInputAction::SetTrue:
            if(it != m_inputValues.end() && it->second == InputState::True)
              return; // no change
            state = InputState::True;
            break;

          case SimulateInputAction::Toggle:
            state = (it == m_inputValues.end() || it->second == InputState::True) ? InputState::False : InputState::True;
            break;

          default:
            assert(false);
            return;
        }
        receive(SetInputState(address, state));
      });
}

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

void Kernel::send(const Message& message)
{
  if(m_ioHandler->send(message))
  {
    if(m_config.debugLogRXTX && (message != Heartbeat() || m_config.debugLogHeartbeat))
      EventLoop::call(
        [this, msg=toString(message)]()
        {
          Log::log(m_logId, LogMessage::D2001_TX_X, msg);
        });
  }
  else
  {} // log message and go to error state
}

void Kernel::restartHeartbeatTimeout()
{
  m_heartbeatTimeout.expires_after(m_config.heartbeatTimeout);
  m_heartbeatTimeout.async_wait(std::bind(&Kernel::heartbeatTimeoutExpired, this, std::placeholders::_1));
}

void Kernel::heartbeatTimeoutExpired(const boost::system::error_code& ec)
{
  if(ec)
    return;
  m_heartbeatTimeout.cancel();
  send(Heartbeat());
  restartHeartbeatTimeout();
}

std::shared_ptr<Decoder> Kernel::getDecoder(uint16_t address, bool longAddress) const
{
  const auto& decoderList = *m_world.decoders;
  std::shared_ptr<Decoder> decoder = decoderList.getDecoder(longAddress ? DecoderProtocol::DCCLong : DecoderProtocol::DCCShort, address);
  if(!decoder)
    decoder = decoderList.getDecoder(address);
  return decoder;
}

void Kernel::throttleSubscribe(uint16_t throttleId, std::pair<uint16_t, bool> key)
{
  auto [unused, added] = m_throttleSubscriptions[throttleId].insert(key);
  if(added)
  {
    EventLoop::call(
      [this, key]()
      {
        if(auto it = m_decoderSubscriptions.find(key); it == m_decoderSubscriptions.end())
        {
          if(auto decoder = getDecoder(key.first, key.second))
            m_decoderSubscriptions.emplace(key, DecoderSubscription{decoder->decoderChanged.connect(std::bind(&Kernel::throttleDecoderChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), 1});
        }
        else
        {
          it->second.count++;
        }
      });
  }
}

void Kernel::throttleUnsubscribe(uint16_t throttleId, std::pair<uint16_t, bool> key)
{
  {
    auto& subscriptions = m_throttleSubscriptions[throttleId];
    subscriptions.erase(key);
    if(subscriptions.empty())
      m_throttleSubscriptions.erase(throttleId);
  }

  EventLoop::call(
    [this, key]()
    {
      if(auto it = m_decoderSubscriptions.find(key); it != m_decoderSubscriptions.end())
      {
        assert(it->second.count > 0);
        if(--it->second.count == 0)
        {
          it->second.connection.disconnect();
          m_decoderSubscriptions.erase(it);
        }
      }
    });
}

void Kernel::throttleDecoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  const std::pair<uint16_t, bool> key(decoder.address, decoder.protocol == DecoderProtocol::DCCLong);

  if(has(changes, DecoderChangeFlags::Direction | DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::SpeedSteps | DecoderChangeFlags::Throttle))
  {
    const bool emergencyStop = decoder.emergencyStop.value();

    uint8_t speedMax = 0;
    if(!emergencyStop)
    {
      speedMax = decoder.speedSteps.value();
      if(speedMax == Decoder::speedStepsAuto)
        speedMax = std::numeric_limits<uint8_t>::max();
    }

    m_ioContext.post(
      [this,
        key,
        direction=decoder.direction.value(),
        speed=speedMax > 0 ? Decoder::throttleToSpeedStep(decoder.throttle, speedMax) : 0,
        speedMax]()
      {
        for(const auto& it : m_throttleSubscriptions)
          if(it.second.count(key) != 0)
            send(ThrottleSetSpeedDirection(it.first, key.first, key.second, speed, speedMax, direction));
      });
  }

  if(has(changes, DecoderChangeFlags::FunctionValue))
  {
    assert(functionNumber <= std::numeric_limits<uint8_t>::max());

    m_ioContext.post(
      [this,
        key,
        number=static_cast<uint8_t>(functionNumber),
        value=decoder.getFunctionValue(functionNumber)]()
      {
        for(const auto& it : m_throttleSubscriptions)
          if(it.second.count(key) != 0)
            send(ThrottleSetFunction(it.first, key.first, key.second, number, value));
      });
  }
}

}
