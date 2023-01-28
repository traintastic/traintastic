/**
 * server/src/hardware/protocol/marklincan/kernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../utils/setthreadname.hpp"

namespace MarklinCAN {

Kernel::Kernel(const Config& config, bool simulation)
  : m_ioContext{1}
  , m_simulation{simulation}
  , m_config{config}
#ifndef NDEBUG
  , m_started{false}
#endif
{
  assert(isEventLoopThread());
  (void)m_simulation;
}

void Kernel::setConfig(const Config& config)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, newConfig=config]()
    {
      m_config = newConfig;
    });
}

void Kernel::setOnStarted(std::function<void()> callback)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_onStarted = std::move(callback);
}

void Kernel::start()
{
  assert(isEventLoopThread());
  assert(m_ioHandler);
  assert(!m_started);

  m_thread = std::thread(
    [this]()
    {
      setThreadName("marklin_can");
      auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
      m_ioContext.run();
    });

  m_ioContext.post(
    [this]()
    {
      m_ioHandler->start();

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
  assert(isEventLoopThread());

  m_ioContext.post(
    [this]()
    {
      m_ioHandler->stop();
    });

  m_ioContext.stop();

  m_thread.join();

#ifndef NDEBUG
  m_started = false;
#endif
}

void Kernel::receive(const Message& message)
{
  assert(isKernelThread());

  if(m_config.debugLogRXTX)
    EventLoop::call([this, msg=toString(message)](){ Log::log(m_logId, LogMessage::D2002_RX_X, msg); });

  switch(message.command())
  {
    case Command::System:
    {
      const auto& system = static_cast<const SystemMessage&>(message);

      switch(system.subCommand())
      {
        case SystemSubCommand::SystemStop:
        case SystemSubCommand::SystemGo:
        case SystemSubCommand::SystemHalt:
        case SystemSubCommand::LocomotiveEmergencyStop:
        case SystemSubCommand::LocomotiveCycleEnd:
        case SystemSubCommand::Overload:
        case SystemSubCommand::Status:
          // not (yet) implemented
          break;
      }
      break;
    }
    case Command::Discovery:
    case Command::Bind:
    case Command::Verify:
    case Command::LocomotiveSpeed:
    case Command::LocomotiveDirection:
    case Command::LocomotiveFunction:
    case Command::ReadConfig:
    case Command::WriteConfig:
    case Command::AccessoryControl:
    case Command::Ping:
      // not (yet) implemented
      break;
  }
}

void Kernel::systemStop()
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this]()
    {
      send(SystemStop());
    });
}

void Kernel::systemGo()
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this]()
    {
      send(SystemGo());
    });
}

void Kernel::systemHalt()
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this]()
    {
        send(SystemHalt());
    });
}

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(isEventLoopThread());
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

void Kernel::send(const Message& message)
{
  assert(isKernelThread());

  if(m_config.debugLogRXTX)
    EventLoop::call([this, msg=toString(message)](){ Log::log(m_logId, LogMessage::D2001_TX_X, msg); });

  m_ioHandler->send(message);
}

void Kernel::postSend(const Message& message)
{
  m_ioContext.post(
    [this, message]()
    {
      send(message);
    });
}

}
