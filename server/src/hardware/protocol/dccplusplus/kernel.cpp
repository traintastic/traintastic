/**
 * server/src/hardware/protocol/dccplusplus/kernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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
#include "../../../utils/setthreadname.hpp"
#include "../../../utils/string.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"

namespace DCCPlusPlus {

Kernel::Kernel(const Config& config)
  : m_ioContext{1}
  , m_startupDelayTimer{m_ioContext}
  , m_decoderController{nullptr}
  , m_config{config}
#ifndef NDEBUG
  , m_started{false}
#endif
{
}

void Kernel::setConfig(const Config& config)
{
  m_ioContext.post(
    [this, newConfig=config]()
    {
      if(newConfig.useEx && newConfig.speedSteps != m_config.speedSteps)
        send(Ex::setSpeedSteps(newConfig.speedSteps));

      m_config = newConfig;
    });
}

void Kernel::start()
{
  assert(m_ioHandler);
  assert(!m_started);

  // reset all state values
  m_powerOn = TriState::Undefined;
  m_emergencyStop = TriState::Undefined;

  m_thread = std::thread(
    [this]()
    {
      setThreadName("dcc++");
      auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
      m_ioContext.run();
    });

  m_ioContext.post(
    [this]()
    {
      m_ioHandler->start();

      m_startupDelayTimer.expires_after(boost::asio::chrono::milliseconds(m_config.startupDelay));
      m_startupDelayTimer.async_wait(std::bind(&Kernel::startupDelayExpired, this, std::placeholders::_1));
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
      m_startupDelayTimer.cancel();

      m_ioHandler->stop();
    });

  m_ioContext.stop();

  m_thread.join();

#ifndef NDEBUG
  m_started = false;
#endif
}

void Kernel::receive(std::string_view message)
{
  if(m_config.debugLogRXTX)
    EventLoop::call(
      [this, msg=std::string(rtrim(message, '\n'))]()
      {
        Log::log(m_logId, LogMessage::D2002_RX_X, msg);
      });

  if(message.size() > 1 && message[0] == '<')
  {
    switch(message[1])
    {
      case 'p': // Power on/off response
        if(message[2] == '0')
        {
          if(m_powerOn != TriState::False)
          {
            m_powerOn = TriState::False;

            if(m_onPowerOnChanged)
              EventLoop::call(
                [this]()
                {
                  m_onPowerOnChanged(false);
                });
          }
        }
        else if(message[2] == '1')
        {
          if(m_powerOn != TriState::True)
          {
            m_powerOn = TriState::True;

            if(m_onPowerOnChanged)
              EventLoop::call(
                [this]()
                {
                  m_onPowerOnChanged(true);
                });
          }
        }
        break;
    }
  }
}

void Kernel::powerOn()
{
  m_ioContext.post(
    [this]()
    {
      if(m_powerOn != TriState::True)
      {
        send(Ex::powerOn());
      }
    });
}

void Kernel::powerOff()
{
  m_ioContext.post(
    [this]()
    {
      if(m_powerOn != TriState::False)
      {
        send(Ex::powerOff());
      }
    });
}

void Kernel::emergencyStop()
{
  m_ioContext.post(
    [this]()
    {
      if(m_emergencyStop != TriState::True)
      {
        m_emergencyStop = TriState::True;
        send(Ex::emergencyStop());
      }
    });
}

void Kernel::clearEmergencyStop()
{
  m_ioContext.post(
    [this]()
    {
      m_emergencyStop = TriState::False;
    });
}

void Kernel::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Throttle | DecoderChangeFlags::Direction))
  {
    const uint8_t speed = Decoder::throttleToSpeedStep(decoder.throttle, 126);
    m_ioContext.post(
      [this, address=decoder.address.value(), emergencyStop=decoder.emergencyStop.value(), speed, direction=decoder.direction.value()]()
      {
        send(Ex::setLocoSpeedAndDirection(address, speed, emergencyStop | (m_emergencyStop != TriState::False), direction));
      });
  }
  else if(has(changes, DecoderChangeFlags::FunctionValue) && functionNumber <= Config::functionNumberMax)
  {
    postSend(Ex::setLocoFunction(decoder.address, static_cast<uint8_t>(functionNumber), decoder.getFunctionValue(functionNumber)));
  }
}

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

void Kernel::send(std::string_view message)
{
  if(m_ioHandler->send(message))
  {
    if(m_config.debugLogRXTX)
      EventLoop::call(
        [this, msg=std::string(rtrim(message, '\n'))]()
        {
          Log::log(m_logId, LogMessage::D2001_TX_X, msg);
        });
  }
  else
  {} // log message and go to error state
}

void Kernel::startupDelayExpired(const boost::system::error_code& ec)
{
  if(ec)
    return;

  m_onStarted();
}

}
