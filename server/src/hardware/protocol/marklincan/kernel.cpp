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
#include "uid.hpp"
#include "../dcc/dcc.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../decoder/decodercontroller.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../utils/inrange.hpp"
#include "../../../utils/setthreadname.hpp"

namespace MarklinCAN {

static std::tuple<bool, DecoderProtocol, uint16_t> uidToProtocolAddress(uint32_t uid)
{
  if(inRange(uid, UID::Range::locomotiveMotorola))
    return {true, DecoderProtocol::Motorola, uid - UID::Range::locomotiveMotorola.first};
  if(inRange(uid, UID::Range::locomotiveDCC))
    return {true, DecoderProtocol::DCC, uid - UID::Range::locomotiveDCC.first};

  return {false, DecoderProtocol::Auto, 0};
}

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

void Kernel::setDecoderController(DecoderController* decoderController)
{
  assert(!m_started);
  m_decoderController = decoderController;
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
          // not (yet) implemented
          break;

        case SystemSubCommand::LocomotiveEmergencyStop:
          if(m_decoderController && system.isResponse())
          {
            auto [success, protocol, address] = uidToProtocolAddress(system.uid());
            if(success)
            {
              EventLoop::call(
                [this, protocol=protocol, address=address]()
                {
                  const bool longAddress = (protocol == DecoderProtocol::DCC) && DCC::isLongAddress(address);
                  if(const auto& decoder = m_decoderController->getDecoder(protocol, address, longAddress))
                    decoder->emergencyStop.setValueInternal(true);
                });
            }
          }
          break;

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
      // not (yet) implemented
      break;

    case Command::LocomotiveSpeed:
      if(m_decoderController)
      {
        const auto& locomotiveSpeed = static_cast<const LocomotiveSpeed&>(message);
        if(locomotiveSpeed.isResponse() && locomotiveSpeed.hasSpeed())
        {
          auto [success, protocol, address] = uidToProtocolAddress(locomotiveSpeed.uid());
          if(success)
          {
            EventLoop::call(
              [this, protocol=protocol, address=address, throttle=Decoder::speedStepToThrottle(locomotiveSpeed.speed(), LocomotiveSpeed::speedMax)]()
              {
                const bool longAddress = (protocol == DecoderProtocol::DCC) && DCC::isLongAddress(address);
                if(const auto& decoder = m_decoderController->getDecoder(protocol, address, longAddress))
                {
                  decoder->emergencyStop.setValueInternal(false);
                  decoder->throttle.setValueInternal(throttle);
                }
              });
          }
        }
      }
      break;

    case Command::LocomotiveDirection:
      if(m_decoderController)
      {
        const auto& locomotiveDirection = static_cast<const LocomotiveDirection&>(message);
        if(locomotiveDirection.isResponse() && locomotiveDirection.hasDirection())
        {
          auto [success, protocol, address] = uidToProtocolAddress(locomotiveDirection.uid());
          if(success)
          {
            Direction direction = Direction::Unknown;
            switch(locomotiveDirection.direction())
            {
              case LocomotiveDirection::Direction::Forward:
                direction = Direction::Forward;
                break;

              case LocomotiveDirection::Direction::Reverse:
                direction = Direction::Reverse;
                break;

              case LocomotiveDirection::Direction::Same:
              case LocomotiveDirection::Direction::Inverse:
                break;
            }

            EventLoop::call(
              [this, protocol=protocol, address=address, direction]()
              {
                const bool longAddress = (protocol == DecoderProtocol::DCC) && DCC::isLongAddress(address);
                if(const auto& decoder = m_decoderController->getDecoder(protocol, address, longAddress))
                  decoder->direction.setValueInternal(direction);
              });
          }
        }
      }
      break;

    case Command::LocomotiveFunction:
      if(m_decoderController)
      {
        const auto& locomotiveFunction = static_cast<const LocomotiveFunction&>(message);
        if(locomotiveFunction.isResponse() && locomotiveFunction.hasValue())
        {
          auto [success, protocol, address] = uidToProtocolAddress(locomotiveFunction.uid());
          if(success)
          {
            EventLoop::call(
              [this, protocol=protocol, address=address, number=locomotiveFunction.number(), value=locomotiveFunction.isOn()]()
              {
                const bool longAddress = (protocol == DecoderProtocol::DCC) && DCC::isLongAddress(address);
                if(const auto& decoder = m_decoderController->getDecoder(protocol, address, longAddress))
                  decoder->setFunctionValue(number, value);
              });
          }
        }
      }
      break;

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

void Kernel::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  assert(isEventLoopThread());
  uint32_t uid = 0;

  switch(decoder.protocol.value())
  {
    case DecoderProtocol::DCC:
      uid = UID::locomotiveDCC(decoder.address);
      break;

    case DecoderProtocol::Motorola:
      uid = UID::locomotiveMotorola(decoder.address);
      break;

    case DecoderProtocol::Auto:
    case DecoderProtocol::Selectrix:
    case DecoderProtocol::Custom:
      break;
  }

  if(uid == 0)
    return;

  if(has(changes, DecoderChangeFlags::Direction))
  {
    LocomotiveDirection::Direction direction = LocomotiveDirection::Direction::Same;

    switch(decoder.direction.value())
    {
      case Direction::Forward:
        direction = LocomotiveDirection::Direction::Forward;
        break;

      case Direction::Reverse:
        direction = LocomotiveDirection::Direction::Reverse;
        break;

      case Direction::Unknown:
        break;
    }

    if(direction != LocomotiveDirection::Direction::Same)
      postSend(LocomotiveDirection(uid, direction));
  }

  if(has(changes, DecoderChangeFlags::EmergencyStop) && decoder.emergencyStop)
    postSend(LocomotiveEmergencyStop(uid));
  else if(has(changes, DecoderChangeFlags::Throttle | DecoderChangeFlags::EmergencyStop))
    postSend(LocomotiveSpeed(uid, Decoder::throttleToSpeedStep(decoder.throttle, LocomotiveSpeed::speedMax)));

  if(has(changes, DecoderChangeFlags::FunctionValue) && functionNumber <= std::numeric_limits<uint8_t>::max())
    postSend(LocomotiveFunction(uid, functionNumber, decoder.getFunctionValue(functionNumber)));
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
