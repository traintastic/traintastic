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
#include "../../input/inputcontroller.hpp"
#include "../../output/outputcontroller.hpp"
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

  send(GetVersion());
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
    case Command::Version:
      KernelBase::started();
      break;

    case Command::Pong:
    //case Command::InputChanged:
    //case Command::ThrottleSetFunctions:
    //case Command::ThrottleSetSpeedDirection:
      //assert(false); // todo: impement
      break;

    case Command::Ping:
    case Command::GetVersion:
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
    restartPingTimeout();

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

}
