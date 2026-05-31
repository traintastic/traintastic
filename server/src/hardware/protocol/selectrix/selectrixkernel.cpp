/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2023-2026 Reinder Feenstra
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

#include "selectrixkernel.hpp"
#include "selectrixconst.hpp"
#include "selectrixutils.hpp"
#include "iohandler/selectrixsimulationiohandler.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../utils/tohex.hpp"

namespace Selectrix {

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_pollTimer{{
    decltype(m_pollTimer)::value_type{ioContext()},
    decltype(m_pollTimer)::value_type{ioContext()},
    decltype(m_pollTimer)::value_type{ioContext()},
    decltype(m_pollTimer)::value_type{ioContext()}}}
  , m_config{config}
{
  assert(isEventLoopThread());

  m_addresses.emplace(BusAddress{Bus::SX0, Address::trackPower}, BusAddressValue{AddressType::TrackPower, 0, false});
}

Kernel::~Kernel() = default;

void Kernel::setConfig(const Config& config)
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this, newConfig=config]()
    {
      m_config = newConfig;
    });
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

  boost::asio::post(m_ioContext,
    [this]()
    {
      try
      {
        m_ioHandler->start();

        if(m_ioHandler->requiresPolling())
        {
          auto nextPoll = std::chrono::steady_clock::now();
          for(auto addressType : addressTypes)
          {
            m_nextPoll[static_cast<size_t>(addressType)] = nextPoll;
            startPollTimer(addressType);
            nextPoll += std::chrono::milliseconds(100) / addressTypes.size();
          }
        }
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

  boost::asio::post(m_ioContext,
    [this]()
    {
      for(auto addressType : addressTypes)
      {
        m_pollTimer[static_cast<size_t>(addressType)].cancel();
      }
      m_ioHandler->stop();
    });

  m_ioContext.stop();

  m_thread.join();

#ifndef NDEBUG
  m_started = false;
#endif
}

void Kernel::busChanged(Bus bus, uint8_t address, uint8_t value)
{
  if(auto it = m_addresses.find({bus, address}); it != m_addresses.end())
  {
    if((it->second.lastValue != value || !it->second.lastValueValid))
    {
      if(m_config.debugLogRXTX)
      {
        EventLoop::call(
          [this, bus, address, value]()
          {
            Log::log(logId, LogMessage::D2012_CHANGED_SXX_X_X_X, static_cast<uint8_t>(bus), address, toHex(value), value);
          });
      }

      switch(it->second.type)
      {
        case AddressType::TrackPower:
        {
          const bool trackPower = value == TrackPower::on;
          if(m_trackPower != trackPower)
          {
            m_trackPower = toTriState(trackPower);
            if(m_onTrackPowerChanged) /*[[likely]]*/
            {
              EventLoop::call(
                [this, trackPower]()
                {
                  m_onTrackPowerChanged(trackPower);
                });
            }
          }
          break;
        }
        case AddressType::Locomotive:
          assert(bus == Bus::SX0);
          EventLoop::call(
            [this, address, value]()
            {
              if(onLocomotiveChanged) [[likely]]
              {
                onLocomotiveChanged(
                  address,
                  value & Locomotive::speedMask,
                  (value & Locomotive::directionMask) == Locomotive::directionReverse,
                  (value & Locomotive::f0),
                  (value & Locomotive::f1)
                );
              }
            });
          break;

        case AddressType::Feedback:
          EventLoop::call(
            [this, bus, address, value]()
            {
              if(onInputChanged) [[likely]]
              {
                const uint16_t baseAddress = 1 + static_cast<uint16_t>(address) * 8;
                for(uint_least8_t i = 0; i < 8; ++i)
                {
                  onInputChanged(bus, baseAddress + i, (value & (1 << i)));
                }
              }
            });
          break;

        case AddressType::Accessory:
          EventLoop::call(
            [this, bus, address, value]()
            {
              if(onOutputChanged) [[likely]]
              {
                const uint16_t baseAddress = 1 + static_cast<uint16_t>(address) * 4;
                for(uint_least8_t i = 0; i < 4; ++i)
                {
                  onOutputChanged(bus, baseAddress + i, static_cast<OutputPairValue>((value >> (i * 2)) & 0x3));
                }
              }
            });
          break;
      }
      it->second.lastValue = value;
      it->second.lastValueValid = true;
    }
  }
}

void Kernel::setTrackPower(bool enable)
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this, enable]()
    {
      if(m_trackPower != toTriState(enable))
      {
        write(Bus::SX0, Address::trackPower, enable ? TrackPower::on : TrackPower::off);
        m_trackPower = TriState::Undefined;
      }
    });
}

void Kernel::setLocomotive(uint8_t address, uint8_t speed, bool directionReverse, bool f0, bool f1)
{
  assert(isEventLoopThread());

  if(address > Address::locomotiveMax) [[unlikely]]
  {
    assert(false);
    return; // invalid locomotive address
  }

  if(const auto it = m_addresses.find({Bus::SX0, address});
      it == m_addresses.end() ||
      it->second.type != AddressType::Locomotive)
  {
    assert(false);
    return; // not a locomotive address
  }

  uint8_t value = speed & Locomotive::speedMask;
  if(directionReverse)
  {
    value |= Locomotive::directionReverse;
  }
  if(f0)
  {
    value |= Locomotive::f0;
  }
  if(f1)
  {
    value |= Locomotive::f1;
  }

  postWrite(Bus::SX0, address, value);
}

void Kernel::simulateInputChange(Bus bus, uint16_t address, SimulateInputAction action)
{
  assert(isEventLoopThread());

  if(m_simulation)
  {
    boost::asio::post(m_ioContext,
      [this, bus, address, action]()
      {
        const auto it = m_addresses.find({bus, toBusAddress(address)});
        if(it != m_addresses.end() && it->second.type == AddressType::Feedback) // check if bus/address is for input
        {
          static_cast<SimulationIOHandler&>(*m_ioHandler).simulateInputChange(bus, address, action);
        }
      });
  }
}

bool Kernel::setOutput(Bus bus, uint32_t flatAddress, OutputPairValue value)
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this, bus, flatAddress, value]()
    {
      const uint8_t address = Accessory::toBusAddress(flatAddress);
      const auto it = m_addresses.find({bus, address});
      if(it != m_addresses.end() && it->second.type == AddressType::Accessory) // check if bus/address is for output
      {
        const uint8_t shift = 2 * Accessory::toPort(flatAddress);
        const uint8_t mask = 0x3 << shift;
        const uint8_t bits = (it->second.lastValue & ~mask) | (static_cast<uint8_t>(value) << shift);
        write(bus, address, bits);
      }
    });

  return true;
}

void Kernel::addAddress(Bus bus, uint8_t address, AddressType type)
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this, bus, address, type]()
    {
      m_addresses.emplace(BusAddress{bus, address}, BusAddressValue{type, 0x00, false});
    });
}

void Kernel::removeAddress(Bus bus, uint8_t address)
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this, bus, address]()
    {
      m_addresses.erase({bus, address});
    });
}

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(isEventLoopThread());
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

bool Kernel::read(Bus bus, uint8_t address)
{
  assert(isKernelThread());

  if(!m_ioHandler->read(bus, address))
  {
    error();
    return false;
  }

  return true;
}

bool Kernel::write(Bus bus, uint8_t address, uint8_t value)
{
  assert(isKernelThread());

  if(m_config.debugLogRXTX)
  {
    EventLoop::call(
      [this, bus, address, value]()
      {
        Log::log(logId, LogMessage::D2011_WRITE_SXX_X_X_X, static_cast<uint8_t>(bus), address, toHex(value), value);
      });
  }

  if(!m_ioHandler->write(bus, address, value))
  {
    error();
    return false;
  }
  return true;
}

void Kernel::startPollTimer(AddressType addressType)
{
  assert(isKernelThread());
  auto& nextPoll = m_nextPoll[static_cast<size_t>(addressType)];
  auto& pollTimer = m_pollTimer[static_cast<size_t>(addressType)];
  nextPoll += m_config.pollInterval(addressType);
  pollTimer.expires_after(nextPoll - std::chrono::steady_clock::now());
  pollTimer.async_wait(std::bind(&Kernel::poll, this, std::placeholders::_1, addressType));
}

void Kernel::poll(const boost::system::error_code& ec, AddressType addressType)
{
  assert(isKernelThread());

  if(ec)
  {
    return;
  }

  for(const auto& it : m_addresses)
  {
    if(it.second.type == addressType)
    {
      if(!read(it.first.bus, it.first.address))
      {
        return;
      }
    }
  }

  startPollTimer(addressType);
}

}
