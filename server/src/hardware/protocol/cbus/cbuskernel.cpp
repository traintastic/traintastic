/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "cbuskernel.hpp"
#include "cbusmessages.hpp"
#include "cbustostring.hpp"
/*
#include "simulator/cbussimulator.hpp"
*/
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../utils/setthreadname.hpp"

namespace CBUS {

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_config{config}
{
  assert(isEventLoopThread());
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

void Kernel::start()
{
  assert(isEventLoopThread());
  assert(m_ioHandler);

  m_thread = std::thread(
    [this]()
    {
      setThreadName("cbus");
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
}

void Kernel::started()
{
  assert(isKernelThread());

  send(RequestCommandStationStatus());
  send(QueryNodeNumber());

  ::KernelBase::started();
}

void Kernel::receive(uint8_t /*canId*/, const Message& message)
{
  assert(isKernelThread());

  if(m_config.debugLogRXTX)
  {
    EventLoop::call(
      [this, msg=toString(message)]()
      {
        Log::log(logId, LogMessage::D2002_RX_X, msg);
      });
  }

  switch(message.opCode)
  {
    case OpCode::TOF:
      m_trackOn = false;
      if(onTrackOff) [[likely]]
      {
        EventLoop::call(onTrackOff);
      }
      break;

    case OpCode::TON:
      m_trackOn = true;
      if(onTrackOn) [[likely]]
      {
        EventLoop::call(onTrackOn);
      }
      break;

    case OpCode::ESTOP:
      if(onEmergencyStop) [[likely]]
      {
        EventLoop::call(onEmergencyStop);
      }
      break;

    default:
      break;
  }
}

void Kernel::trackOff()
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this]()
    {
      if(m_trackOn)
      {
        send(RequestTrackOff());
      }
    });
}

void Kernel::trackOn()
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this]()
    {
      if(!m_trackOn)
      {
        send(RequestTrackOn());
      }
    });
}

void Kernel::requestEmergencyStop()
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this]()
    {
      send(RequestEmergencyStop());
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
  {
    EventLoop::call(
      [this, msg=toString(message)]()
      {
        Log::log(logId, LogMessage::D2001_TX_X, msg);
      });
  }

  if(auto ec = m_ioHandler->send(message); ec)
  {
    (void)ec; // FIXME: handle error
  }
}

}
