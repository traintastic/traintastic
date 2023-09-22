/**
 * server/src/hardware/protocol/dccplusplus/kernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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
#include "../../input/inputcontroller.hpp"
#include "../../output/outputcontroller.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../utils/rtrim.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../utils/inrange.hpp"
#include "../../../utils/fromchars.hpp"
#include "../../../utils/displayname.hpp"

namespace DCCPlusPlus {

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_startupDelayTimer{m_ioContext}
  , m_decoderController{nullptr}
  , m_inputController{nullptr}
  , m_outputController{nullptr}
  , m_config{config}
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
  m_inputValues.clear();

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
        Log::log(logId, LogMessage::D2002_RX_X, msg);
      });

  if(message.size() > 1 && message[0] == '<')
  {
    switch(message[1])
    {
      case 'H': // Turnout response
      {
        uint16_t id;
        if(auto r = fromChars(message.substr(3), id); r.ec == std::errc())
        {
          const char state = *(r.ptr + 1);
          TriState value = TriState::Undefined;

          if(state == '0' || state == 'C')
            value = TriState::False;
          else if(state == '1' || state == 'T')
            value = TriState::True;

          if(value != TriState::Undefined)
          {
            EventLoop::call(
              [this, id, value]()
              {
                m_outputController->updateOutputValue(OutputChannel::turnout, id, value);
              });
          }
        }
        break;
      }
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

      case 'q': // Sensor/Input: ACTIVE to INACTIVE
      case 'Q': // Sensor/Input: INACTIVE to ACTIVE
        if(m_inputController && message[2] == ' ')
        {
          uint32_t id;
          if(auto r = fromChars(message.substr(3), id); r.ec == std::errc() && *r.ptr == '>' && id <= idMax)
          {
            const bool value = message[1] == 'Q';
            auto it = m_inputValues.find(id);
            if(it == m_inputValues.end() || it->second != value)
            {
              m_inputValues[id] = value;

              EventLoop::call(
                [this, id, value]()
                {
                  m_inputController->updateInputValue(InputController::defaultInputChannel, id, toTriState(value));
                });
            }
          }
        }
        break;

      case 'Y': // Output response
      {
        uint16_t id;
        if(auto r = fromChars(message.substr(3), id); r.ec == std::errc())
        {
          const char state = *(r.ptr + 1);
          TriState value = TriState::Undefined;

          if(state == '0')
            value = TriState::False;
          else if(state == '1')
            value = TriState::True;

          if(value != TriState::Undefined)
          {
            EventLoop::call(
              [this, id, value]()
              {
                m_outputController->updateOutputValue(OutputChannel::output, id, value);
              });
          }
        }
        break;
      }
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
    const uint8_t speed = Decoder::throttleToSpeedStep<uint8_t>(decoder.throttle, 126);
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

bool Kernel::setOutput(uint32_t channel, uint16_t address, bool value)
{
  switch(channel)
  {
    case OutputChannel::dccAccessory:
      assert(inRange<uint32_t>(address, dccAccessoryAddressMin, dccAccessoryAddressMax));
      m_ioContext.post(
        [this, address, value]()
        {
          send(Ex::setAccessory(address, value));

          // no response for accessory command, assume it succeeds:
          EventLoop::call(
            [this, address, value]()
            {
              m_outputController->updateOutputValue(OutputChannel::dccAccessory, address, toTriState(value));
            });
        });
      return true;

    case OutputChannel::turnout:
      assert(inRange<uint32_t>(address, idMin, idMax));
      m_ioContext.post(
        [this, address, value]()
        {
          send(Ex::setTurnout(address, value));
        });
      return true;

    case OutputChannel::output:
      assert(inRange<uint32_t>(address, idMin, idMax));
      m_ioContext.post(
        [this, address, value]()
        {
          send(Ex::setOutput(address, value));
        });
      return true;
  }

  assert(false);
  return false;
}

void Kernel::simulateInputChange(uint16_t address, SimulateInputAction action)
{
  if(m_simulation)
    m_ioContext.post(
      [this, address, action]()
      {
        bool value;
        auto it = m_inputValues.find(address);
        switch(action)
        {
          case SimulateInputAction::SetFalse:
            if(it != m_inputValues.end() && !it->second)
              return; // no change
            value = false;
            break;

          case SimulateInputAction::SetTrue:
            if(it != m_inputValues.end() && it->second)
              return; // no change
            value = true;
            break;

          case SimulateInputAction::Toggle:
            value = it != m_inputValues.end() ? !it->second : true;
            break;

          default:
            assert(false);
            return;
        }
        receive(Ex::sensorTransition(address, value));
      });
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
          Log::log(logId, LogMessage::D2001_TX_X, msg);
        });
  }
  else
  {} // log message and go to error state
}

void Kernel::startupDelayExpired(const boost::system::error_code& ec)
{
  if(ec)
    return;

  started();
}

}
