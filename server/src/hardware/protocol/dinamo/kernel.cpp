/**
 * server/src/hardware/protocol/dinamo/kernel.cpp
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
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../utils/setthreadname.hpp"

namespace Dinamo {

const Kernel::ProtocolVersion Kernel::ProtocolVersion::minimum{3, 0, 0, 0};

static constexpr bool operator >=(const Kernel::ProtocolVersion lhs, const Kernel::ProtocolVersion rhs)
{
  return
    (lhs.major > rhs.major) ||
    ((lhs.major == rhs.major) && (lhs.minor > rhs.minor)) ||
    ((lhs.major == rhs.major) && (lhs.minor == rhs.minor) && (lhs.sub > rhs.sub)) ||
    ((lhs.major == rhs.major) && (lhs.minor == rhs.minor) && (lhs.sub == rhs.sub)) ||
    ((lhs.major == rhs.major) && (lhs.minor == rhs.minor) && (lhs.sub == rhs.sub) && (lhs.sub >= rhs.sub));
}

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_keepAliveTimeout{m_ioContext}
  , m_config{config}
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

void Kernel::setOnFault(OnFault callback)
{
  assert(!m_started);
  m_onFault = std::move(callback);
}

void Kernel::start()
{
  assert(m_ioHandler);
  assert(!m_started);

  m_thread = std::thread(
    [this]()
    {
      setThreadName("dinamo");
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
  if(m_config.debugLogRXTX && (!isNull(message) || m_config.debugLogNull))
  {
    EventLoop::call(
      [this, msg=toString(message)]()
      {
        Log::log(logId, LogMessage::D2002_RX_X, msg);
      });
  }

  const auto fault = toTriState(message.header.isFault());
  if(m_fault != fault)
  {
    m_fault = fault;
    if(m_fault == TriState::True && m_onFault)
    {
      EventLoop::call(
        [this]()
        {
          m_onFault();
        });
    }
  }

  if(message.header.dataSize() >= 1)
  {
    switch(static_cast<const Command&>(message).command)
    {
      case Command::systemControl:
        if(message.header.dataSize() >= 2)
        {
          switch(static_cast<const SubCommand&>(message).subCommand)
          {
            case SubCommand::protocolVersion:
            {
              if(message.size() == sizeof(ProtocolVersionResponse) && m_state == State::ProtocolVersion)
              {
                const auto& response = static_cast<const ProtocolVersionResponse&>(message);
                m_protocolVersion = {response.majorRelease(), response.minorRelease(), response.subRelease(), response.bugFix()};
                if(m_protocolVersion >= ProtocolVersion::minimum)
                {
                  EventLoop::call(
                    [this, version=m_protocolVersion]()
                    {
                      Log::log(logId, LogMessage::I2006_PROTOCOL_VERSION_X, version.toString());
                    });
                  nextState();
                }
                else
                {
                  EventLoop::call(
                    [this, version=m_protocolVersion]()
                    {
                      Log::log(logId, LogMessage::E2025_MINIMUM_PROTOCOL_VERSION_IS_X_GOT_X, ProtocolVersion::minimum.toString(), version.toString());
                    });
                  error();
                }
              }
              break;
            }
          }
        }
        break;
    }
  }
}

void Kernel::setFault()
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this]()
    {
      if(m_fault != TriState::True)
      {
        Null request;
        request.header.setFault(true);
        send(request);
      }
    });
}

void Kernel::resetFault()
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this]()
    {
      if(m_fault != TriState::False)
      {
        ResetFault request;
        send(request);
      }
    });
}

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

void Kernel::send(Message& message)
{
  message.header.setToggle(m_sendToggle);
  m_sendToggle = !m_sendToggle;
  updateChecksum(message);

  if(m_ioHandler->send(message))
  {
    restartKeepAliveTimeout();

    if(m_config.debugLogRXTX && (!isNull(message) || m_config.debugLogNull))
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

void Kernel::restartKeepAliveTimeout()
{
  m_keepAliveTimeout.cancel();
  m_keepAliveTimeout.expires_after(Config::keepAliveTimeout);
  m_keepAliveTimeout.async_wait(
    [this](const boost::system::error_code& ec)
    {
      if(!ec)
      {
        Null null;
        send(null);
      }
    });
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

    case State::ProtocolVersion:
    {
      ProtocolVersionRequest request;
      send(request);
      break;
    }
    case State::Started:
      KernelBase::started();
      restartKeepAliveTimeout();
      break;
  }
}

std::string Kernel::ProtocolVersion::toString() const
{
  auto s = std::to_string(major).append(".").append(std::to_string(minor));
  if(sub != 0)
  {
    s.append(std::to_string(sub));
  }
  if(bugFix != 0)
  {
    const char c = 'a' + bugFix - 1;
    s.append(&c, sizeof(c));
  }
  return s;
}

}
