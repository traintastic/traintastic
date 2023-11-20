/**
 * server/src/hardware/protocol/selectrix/kernel.cpp
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
#include "const.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../utils/tohex.hpp"

namespace Selectrix {

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_config{config}
{
  assert(isEventLoopThread());
  (void)m_simulation; // silence unused warning
}

Kernel::~Kernel() = default;

void Kernel::setConfig(const Config& config)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, newConfig=config]()
    {
      m_config = newConfig;
    });
}

void Kernel::setOnTrackPowerChanged(std::function<void(bool)> callback)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_onTrackPowerChanged = std::move(callback);
}

void Kernel::start()
{
  assert(isEventLoopThread());
  assert(m_ioHandler);
  assert(!m_started);

  // reset all state values
  m_trackPower = TriState::Undefined;

  m_thread = std::thread(
    [this]()
    {
      setThreadName("selectrix");
      auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
      m_ioContext.run();
    });

  m_ioContext.post(
    [this]()
    {
      try
      {
        m_ioHandler->start();

        write(Address::selectSXBus, static_cast<uint8_t>(m_bus));
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

      started();
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

void Kernel::setTrackPower(bool enable)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, enable]()
    {
      if(m_trackPower != toTriState(enable))
      {
        if(write(Bus::SX0, Address::trackPower, enable ? TrackPower::on : TrackPower::off))
        {
          uint8_t value;
          if(read(Bus::SX0, Address::trackPower, value))
          {
            switch(value)
            {
              case TrackPower::off:
                m_trackPower = TriState::False;
                return;

              case TrackPower::on:
                m_trackPower = TriState::True;
                return;
            }
          }
        }
        m_trackPower = TriState::Undefined;
      }
    });
}

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(isEventLoopThread());
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

bool Kernel::selectBus(Bus bus)
{
  if(m_bus == bus)
  {
    return true;
  }

  if(!write(Address::selectSXBus, static_cast<uint8_t>(bus)))
  {
    return false;
  }

  m_bus = bus;

  return true;
}

bool Kernel::read(Bus bus, uint8_t address, uint8_t& value)
{
  if(!selectBus(bus))
  {
    return false;
  }

  const bool success = m_ioHandler->read(address, value);

  if(m_config.debugLogRXTX)
  {
    EventLoop::call(
      [this, bus, address, value]()
      {
        Log::log(logId, LogMessage::D2012_READ_SXX_X_X_X, static_cast<uint8_t>(bus), address, toHex(value), value);
      });
  }

  return success;
}

bool Kernel::write(Bus bus, uint8_t address, uint8_t value)
{
  return selectBus(bus) && write(address, value);
}

bool Kernel::write(uint8_t address, uint8_t value)
{
  if(m_config.debugLogRXTX)
  {
    EventLoop::call(
      [this, bus=m_bus, address, value]()
      {
        Log::log(logId, LogMessage::D2011_WRITE_SXX_X_X_X, static_cast<uint8_t>(bus), address, toHex(value), value);
      });
  }

  return m_ioHandler->write(address, value);
}

}
