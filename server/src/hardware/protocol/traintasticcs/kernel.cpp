/**
 * server/src/hardware/protocol/traintasticcs/kernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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
#include "../../interface/interface.hpp"
#include "../../input/inputcontroller.hpp"
#include "../../output/outputcontroller.hpp"
#include "../../throttle/hardwarethrottle.hpp"
#include "../../../utils/inrange.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../world/world.hpp"

namespace TraintasticCS {

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_pingTimeout{m_ioContext}
  , m_config{config}
{
  (void)m_simulation;
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

  send(GetInfo());
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

  switch(message.command)
  {
    case Command::Info:
    {
      const auto& info = static_cast<const Info&>(message);
      m_info.board = info.board;
      m_info.version.major = info.versionMajor;
      m_info.version.minor = info.versionMinor;
      m_info.version.patch = info.versionPatch;
      m_initialized = true;
      KernelBase::started();
      restartPingTimeout();
      break;
    }
    case Command::Pong:
    //case Command::InputChanged:
      break;

    case Command::ThrottleSetSpeedDirection:
    {
      EventLoop::call(
        [this, setSpeedDirection=static_cast<const ThrottleSetSpeedDirection&>(message)]()
        {
          if(auto throttle = getThrottle(setSpeedDirection.channel, setSpeedDirection.throttleId(), setSpeedDirection.protocol(), setSpeedDirection.address()))
          {
            throttle->emergencyStop = (setSpeedDirection.eStop != 0);

            if(setSpeedDirection.setSpeedStep)
            {
              if(setSpeedDirection.speedStep <= setSpeedDirection.speedSteps && setSpeedDirection.speedSteps != 0)
              {
                throttle->throttle = static_cast<float>(setSpeedDirection.speedStep) / static_cast<float>(setSpeedDirection.speedSteps);
              }
              else
              {
                throttle->throttle = Throttle::throttleStop;
              }
            }

            if(setSpeedDirection.setDirection)
            {
              throttle->direction = (setSpeedDirection.direction != 0) ? Direction::Forward : Direction::Reverse;
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
            for(uint8_t i = 0; i < setFunctions.functionCount(); ++i)
            {
              auto [number, value] = setFunctions.function(i);
              if(auto function = throttle->getFunction(number))
              {
                function->value = value;
              }
            }
          }
        });
      break;
    }
    case Command::Ping:
    case Command::GetInfo:
    //case Command::GetInputState:
      assert(false); // we MUST never receive these
      break;
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
    if(m_initialized)
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
    auto result = throttleInfo->throttle->acquire(protocol, address, steal);
    if(result == Throttle::AcquireResult::Success)
    {
      throttleInfo->protocol = protocol;
      throttleInfo->address = address;
    }
    else
    {
      LOG_DEBUG("acquire failed: %1", (int)result);
      return nullThrottle;
    }
  }

  return throttleInfo->throttle;
}

}
