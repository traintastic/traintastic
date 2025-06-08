/**
 * server/src/hardware/protocol/traintasticcs/kernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024-2025 Reinder Feenstra
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
#include "iohandler/simulationiohandler.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../decoder/list/decoderlist.hpp"
#include "../../interface/interface.hpp"
#include "../../input/inputcontroller.hpp"
#include "../../output/outputcontroller.hpp"
#include "../../../throttle/hardwarethrottle.hpp"
#include "../../../utils/inrange.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../world/world.hpp"

namespace TraintasticCS {

static_assert(Kernel::InputChannel::s88 == static_cast<uint32_t>(InputChannel::S88));

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_responseTimeout{m_ioContext}
  , m_pingTimeout{m_ioContext}
  , m_config{config}
{
}

Kernel::~Kernel() = default;

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

  m_thread = std::thread(
    [this]()
    {
      setThreadName("traintasticcs");
      auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
      m_ioContext.run();
    });

  m_ioContext.post(
    [this]()
    {
      try
      {
        m_ioHandler->start();
      }
      catch(const LogMessageException& e)
      {
        EventLoop::call(
          [this, e]()
          {
            Log::log(logId, e.message(), e.args());
            error();
          });
        return;
      }
    });

#ifndef NDEBUG
  m_started = true;
#endif
}

void Kernel::stop()
{
  m_ioContext.post(
    [this]()
    {
      m_pingTimeout.cancel();
      m_ioHandler->stop();
    });

  m_ioContext.stop();

  m_thread.join();

#ifndef NDEBUG
  m_started = false;
#endif
}

void Kernel::started()
{
  assert(isKernelThread());

  nextState(); // start initalization sequence
}

void Kernel::receive(const Message& message)
{
  if(m_config.debugLogRXTX && (message != Pong() || m_config.debugLogPing))
  {
    EventLoop::call(
      [this, msg=toString(message)]()
      {
        Log::log(logId, LogMessage::D2002_RX_X, msg);
      });
  }

  // remove from response timeout list:
  if(!m_waitingForResponse.empty() && isResponse(message.command))
  {
    const Command command = (message.command == Command::Error)
      ? static_cast<const Error&>(message).request
      : static_cast<Command>(static_cast<std::underlying_type_t<Command>>(message.command) & 0x7F);

    auto it = m_waitingForResponse.begin();
    while(it->first != command && it != m_waitingForResponse.end())
    {
      it++;
    }
    if(it != m_waitingForResponse.end())
    {
      const bool restartTimer = (it == m_waitingForResponse.begin());
      m_waitingForResponse.erase(it);
      if(restartTimer && !m_waitingForResponse.empty())
      {
        restartResponseTimeoutTimer();
      }
      else
      {
        m_responseTimeout.cancel();
      }
    }
  }

  switch(message.command)
  {
    case Command::ResetOk:
      if(m_state == State::Reset)
      {
        nextState();
      }
      else
      {
        assert(false); // may not happen, reset only once at start up
      }
      break;

    case Command::Info:
      if(m_state == State::GetInfo)
      {
        const auto& info = static_cast<const Info&>(message);
        m_info.board = info.board;
        m_info.version.major = info.versionMajor;
        m_info.version.minor = info.versionMinor;
        m_info.version.patch = info.versionPatch;
        nextState();
      }
      else
      {
        assert(false); // may not happen, get info only once at start up
      }
      break;

    case Command::Pong:
      break;

    case Command::InitXpressNetOk:
      if(m_state == State::InitXpressNet) /*[[likely]]*/
      {
        nextState();
      }
      else
      {
        assert(false); // may not happen, init xpressnet only once at start up
      }
      break;

    case Command::InitS88Ok:
      if(m_state == State::InitS88) /*[[likely]]*/
      {
        nextState();
      }
      else
      {
        assert(false); // may not happen, init S88 only once at start up
      }
      break;

    case Command::InitLocoNetOk:
      if(m_state == State::InitLocoNet) /*[[likely]]*/
      {
        nextState();
      }
      else
      {
        assert(false); // may not happen, init LocoNet only once at start up
      }
      break;

    case Command::InputStateChanged:
      if(m_inputController && message.size() == sizeof(InputStateChanged))
      {
        EventLoop::call(
          [this, inputStateChanged=static_cast<const InputStateChanged&>(message)]()
          {
            m_inputController->updateInputValue(static_cast<uint32_t>(inputStateChanged.channel), inputStateChanged.address(), toTriState(inputStateChanged.state));
          });
      }
      break;

    case Command::ThrottleSetSpeedDirection:
    {
      EventLoop::call(
        [this, setSpeedDirection=static_cast<const ThrottleSetSpeedDirection&>(message)]()
        {
          if(auto throttle = getThrottle(setSpeedDirection.channel, setSpeedDirection.throttleId(), setSpeedDirection.protocol(), setSpeedDirection.address()))
          {
            if(setSpeedDirection.eStop != 0)
            {
              throttle->emergencyStop();
            }
            else if(setSpeedDirection.setSpeedStep)
            {
              if(setSpeedDirection.speedStep <= setSpeedDirection.speedSteps && setSpeedDirection.speedSteps != 0)
              {
                throttle->setSpeedRatio(static_cast<double>(setSpeedDirection.speedStep) / static_cast<double>(setSpeedDirection.speedSteps));
              }
              else
              {
                throttle->setSpeedRatio(0.0);
              }
            }

            if(setSpeedDirection.setDirection)
            {
              throttle->setDirection((setSpeedDirection.direction != 0) ? Direction::Forward : Direction::Reverse);
            }
          }
        });
      break;
    }
    case Command::ThrottleSetFunctions:
    {
      const std::vector<std::byte> buffer(reinterpret_cast<const std::byte*>(&message), reinterpret_cast<const std::byte*>(&message) + message.size());
      EventLoop::call(
        [this, buffer]()
        {
          const auto& setFunctions = *reinterpret_cast<const ThrottleSetFunctions*>(buffer.data());
          if(auto throttle = getThrottle(setFunctions.channel, setFunctions.throttleId(), setFunctions.protocol(), setFunctions.address()))
          {
            if(const auto& decoder = throttle->getDecoder(setFunctions.protocol(), setFunctions.address()))
            {
              for(uint8_t i = 0; i < setFunctions.functionCount(); ++i)
              {
                auto [number, value] = setFunctions.function(i);
                if(auto function = decoder->getFunction(number))
                {
                  function->value = value;
                }
              }
            }
          }
        });
      break;
    }
    case Command::Error:
      EventLoop::call(
        [this, errMsg=static_cast<const Error&>(message)]()
        {
          Log::log(logId, LogMessage::E2025_PROTOCOL_ERROR_X_X_FOR_COMMAND_X_X,
            static_cast<std::underlying_type_t<ErrorCode>>(errMsg.code), toString(errMsg.code),
            static_cast<std::underlying_type_t<ErrorCode>>(errMsg.request), toString(errMsg.request));
          error();
        });
      break;

    case Command::Reset:
    case Command::Ping:
    case Command::GetInfo:
    case Command::InitXpressNet:
    case Command::InitS88:
    case Command::InitLocoNet:
      assert(false); // we MUST never receive these
      break;
  }
}

void Kernel::inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action)
{
  if(m_simulation)
  {
    ioHandler<SimulationIOHandler>().inputSimulateChange(static_cast<TraintasticCS::InputChannel>(channel), address, action);
  }
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
    startResponseTimeout(message.command);

    if(m_state == State::Started)
    {
      restartPingTimeout();
    }

    if(m_config.debugLogRXTX && (message != Ping() || m_config.debugLogPing))
    {
      EventLoop::call(
        [this, msg=toString(message)]()
        {
          Log::log(logId, LogMessage::D2001_TX_X, msg);
        });
    }
  }
  else
  {
    // log message and go to error state
    error();
  }
}

void Kernel::startResponseTimeout(Command command)
{
  assert(isKernelThread());

  // determine expire
  std::chrono::steady_clock::time_point expire;
  switch(command)
  {
    case Command::Reset:
      expire = std::chrono::steady_clock::now() + Config::resetResponseTimeout;
      break;

    case Command::Ping:
    case Command::GetInfo:
    case Command::InitXpressNet:
    case Command::InitS88:
    case Command::InitLocoNet:
      expire = std::chrono::steady_clock::now() + Config::responseTimeout;
      break;

    default: /*[[unlikely]]*/
      assert(false);
      return;
  }

  // insert in sorted list:
  auto it = m_waitingForResponse.begin();
  while(it != m_waitingForResponse.end() && it->second <= expire)
  {
    it++;
  }
  const bool insertedAtFront = (it == m_waitingForResponse.begin());
  m_waitingForResponse.emplace(it, std::make_pair(command, expire));

  if(insertedAtFront)
  {
    restartResponseTimeoutTimer();
  }
}

void Kernel::restartResponseTimeoutTimer()
{
  assert(!m_waitingForResponse.empty());
  m_responseTimeout.cancel();
  m_responseTimeout.expires_at(m_waitingForResponse.front().second);
  m_responseTimeout.async_wait(
    [this](const boost::system::error_code& ec)
    {
      if(!ec)
      {
        EventLoop::call(
          [this]()
          {
            Log::log(logId, LogMessage::E2019_TIMEOUT_NO_RESPONSE_WITHIN_X_MS, Config::responseTimeout.count());
            error();
          });
      }
    });
}

void Kernel::restartPingTimeout()
{
  m_pingTimeout.cancel();
  m_pingTimeout.expires_after(Config::pingTimeout);
  m_pingTimeout.async_wait(
    [this](const boost::system::error_code& ec)
    {
      if(!ec)
      {
        send(Ping());
        restartPingTimeout();
      }
    });
}

const std::shared_ptr<HardwareThrottle>& Kernel::getThrottle(ThrottleChannel channel, uint16_t throttleId, DecoderProtocol protocol, uint16_t address, bool steal)
{
  assert(isEventLoopThread());

  static const std::shared_ptr<HardwareThrottle> nullThrottle;

  ThrottleInfo* throttleInfo = nullptr;

  const auto key = std::make_pair(channel, throttleId);
  auto it = m_throttles.find(key);
  if(it != m_throttles.end())
  {
    throttleInfo = &it->second;
  }
  else if(auto* interface = dynamic_cast<Interface*>(m_throttleController))
  {
    auto throttle = HardwareThrottle::create(std::dynamic_pointer_cast<ThrottleController>(interface->shared_ptr<Interface>()), interface->world());
    throttle->name.setValueInternal(std::string(::toString(channel)).append(" #").append(std::to_string(throttleId)));
    auto [emplaceIt, success] = m_throttles.emplace(key, ThrottleInfo{DecoderProtocol::None, 0, std::move(throttle)});
    assert(success);
    throttleInfo = &emplaceIt->second;
  }

  if(!throttleInfo) /*[[unlikely]]*/
  {
    assert(false);
    return nullThrottle;
  }

  assert(throttleInfo->throttle);

  if(protocol != throttleInfo->protocol ||
      address != throttleInfo->address ||
      !throttleInfo->throttle->acquired())
  {
    if(throttleInfo->throttle->acquired())
    {
      throttleInfo->throttle->release();
    }
    auto ec = throttleInfo->throttle->acquire(protocol, address, steal);
    if(!ec)
    {
      throttleInfo->protocol = protocol;
      throttleInfo->address = address;
    }
    else
    {
      LOG_DEBUG("acquire failed: %1", ec.message());
      return nullThrottle;
    }
  }

  return throttleInfo->throttle;
}

void Kernel::changeState(State value)
{
  assert(isKernelThread());
  assert(m_state != value);

  m_state = value;

  switch(m_state)
  {
    case State::Initial:
      break;

    case State::Reset:
      send(Reset());
      break;

    case State::GetInfo:
      send(GetInfo());
      break;

    case State::InitLocoNet:
      if(m_config.loconet.enabled)
      {
        send(InitLocoNet());
      }
      else
      {
        nextState();
      }
      break;

    case State::InitXpressNet:
      if(m_config.xpressnet.enabled)
      {
        send(InitXpressNet());
      }
      else
      {
        nextState();
      }
      break;

    case State::InitS88:
      if(m_config.s88.enabled)
      {
        send(InitS88(m_config.s88.moduleCount, m_config.s88.clockFrequency));
      }
      else
      {
        nextState();
      }
      break;

    case State::Started:
      KernelBase::started();
      restartPingTimeout(); // enable keep alive ping
      break;
  }
}

}
