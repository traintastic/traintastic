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
#include "utils.hpp"
#include "iohandler/simulationiohandler.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../decoder/decoder.hpp"
#include "../../input/inputcontroller.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../utils/tohex.hpp"

namespace Selectrix {

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_pollTimer{ioContext()}
  , m_config{config}
{
  assert(isEventLoopThread());
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

void Kernel::setDecoderController(DecoderController* decoderController)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_decoderController = decoderController;
}

void Kernel::setInputController(InputController* inputController)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_inputController = inputController;
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

        m_nextPoll = std::chrono::steady_clock::now();
        startPollTimer();
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
      m_pollTimer.cancel();
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

void Kernel::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  assert(isEventLoopThread());

  if(has(changes, DecoderChangeFlags::FunctionValue) && functionNumber > 1)
  {
    return; // Selectrix only supports two functions.
  }

  if(decoder.address > Address::locomotiveMax) /*[[unlikely]]*/
  {
    return;
  }

  uint8_t value = 0x00;

  if(!decoder.emergencyStop)
  {
    value |= Decoder::throttleToSpeedStep(decoder.throttle, Locomotive::speedStepMax);
  }
  if(decoder.direction == Direction::Forward)
  {
    value |= Locomotive::directionForward;
  }
  if(decoder.getFunctionValue(0))
  {
    value |= Locomotive::f0;
  }
  if(decoder.getFunctionValue(1))
  {
    value |= Locomotive::f1;
  }

  postWrite(Bus::SX0, static_cast<uint8_t>(decoder.address.value()), value);
}

void Kernel::simulateInputChange(Bus bus, uint16_t address, SimulateInputAction action)
{
  assert(isEventLoopThread());

  if(m_simulation)
  {
    m_ioContext.post(
      [this, bus, address, action]()
      {
        auto it = std::find_if(m_pollAddresses.begin(), m_pollAddresses.end(),
          [bus, busAddress=toBusAddress(address)](const auto& info)
          {
            return bus == info.bus && busAddress == info.address;
          });

        if(it != m_pollAddresses.end() && it->type == AddressType::Input) // check if bus/address is for input
        {
          static_cast<SimulationIOHandler&>(*m_ioHandler).simulateInputChange(bus, address, action);
        }
      });
  }
}

void Kernel::addPollAddress(Bus bus, uint8_t address, AddressType type)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, bus, address, type]()
    {
      // insert sorted on bus (minimize number of bus select commands)
      auto it = std::upper_bound(m_pollAddresses.begin(), m_pollAddresses.end(), bus,
        [](Bus b, const PollInfo& info)
        {
          return b < info.bus;
        });
      m_pollAddresses.emplace(it, PollInfo{bus, address, type, 0x00, false});
    });
}

void Kernel::removePollAddress(Bus bus, uint8_t address)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, bus, address]()
    {
      auto it = std::find_if(m_pollAddresses.begin(), m_pollAddresses.end(),
        [bus, address](const auto& info)
        {
          return bus == info.bus && address == info.address;
        });

      if(it != m_pollAddresses.end()) /*[[likely]]*/
      {
        m_pollAddresses.erase(it);
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
  assert(isKernelThread());

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
  assert(isKernelThread());

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
  assert(isKernelThread());

  return selectBus(bus) && write(address, value);
}

bool Kernel::write(uint8_t address, uint8_t value)
{
  assert(isKernelThread());

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

void Kernel::startPollTimer()
{
  assert(isKernelThread());
  m_nextPoll += m_config.pollInterval;
  m_pollTimer.expires_after(m_nextPoll - std::chrono::steady_clock::now());
  m_pollTimer.async_wait(std::bind(&Kernel::poll, this, std::placeholders::_1));
}

void Kernel::poll(const boost::system::error_code& ec)
{
  assert(isKernelThread());

  if(ec)
    return;

  uint8_t value;

  // read track power status:
  if(read(Bus::SX0, Address::trackPower, value))
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
  }

  for(auto& pollInfo : m_pollAddresses)
  {
    if(read(pollInfo.bus, pollInfo.address, value) && (pollInfo.lastValue != value || !pollInfo.lastValueValid))
    {
      switch(pollInfo.type)
      {
        case AddressType::Locomotive:
          break;

        case AddressType::Input:
          EventLoop::call(
            [this, bus=pollInfo.bus, address=pollInfo.address, value]()
            {
              if(m_inputController) /*[[likely]]*/
              {
                const uint32_t channel = 1 + static_cast<uint8_t>(bus);
                const uint32_t baseAddress = 1 + static_cast<uint32_t>(address) * 8;

                for(uint_least8_t i = 0; i < 8; ++i)
                {
                  m_inputController->updateInputValue(channel, baseAddress + i, toTriState(value & (1 << i)));
                }
              }
            });
          break;
      }
      pollInfo.lastValue = value;
      pollInfo.lastValueValid = true;
    }
  }

  startPollTimer();
}

}
