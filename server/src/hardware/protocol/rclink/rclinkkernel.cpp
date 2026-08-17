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

#include "rclinkkernel.hpp"
#include "rclinkmessages.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../utils/inrange.hpp"
#include "../../../utils/setthreadname.hpp"

namespace RCLink {

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_config{config}
{
  assert(isEventLoopThread());

  std::fill(m_railcomAddress.begin(), m_railcomAddress.end(), 0);
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
      setThreadName("rclink");
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

  send(InitializeAll());

  send(QueryInfo());

  ::KernelBase::started();
}

void Kernel::receive(std::span<const uint8_t> message)
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

  if(Detector::check(message))
  {
    EventLoop::call(
      [this, detector=*reinterpret_cast<const Detector*>(message.data())]()
      {
        if(onDetector && inRange(detector.address, inputAddressMin, inputAddressMax)) [[likely]]
        {
          uint16_t railcomAddress = 0;
          if(detector.occupied() && detector.hasRailcomAddress())
          {
            railcomAddress = detector.railcomAddress();
            m_railcomAddress[detector.address - inputAddressMin] = railcomAddress;
          }
          else if(!detector.occupied())
          {
            railcomAddress = m_railcomAddress[detector.address - inputAddressMin];
            m_railcomAddress[detector.address - inputAddressMin] = 0;
          }
          onDetector(detector.address, detector.occupied(), railcomAddress, detector.orientation());
        }
      });
  }
}

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(isEventLoopThread());
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

template<typename T>
void Kernel::send(const T& message)
{
  static_assert(std::is_base_of_v<Message, T>);
  assert(isKernelThread());
  const auto* p = reinterpret_cast<const uint8_t*>(&message);
  send({p, p + sizeof(message)});
}

void Kernel::send(std::span<const uint8_t> message)
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
