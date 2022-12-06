/**
 * server/src/hardware/protocol/withrottle/kernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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
#include <traintastic/enum/decoderprotocol.hpp>
#include "messages.hpp"
#include "../../interface/interface.hpp"
#include "../../throttle/hardwarethrottle.hpp"
#include "../../throttle/throttlecontroller.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../clock/clock.hpp"
#include "../../../log/log.hpp"
#include "../../../utils/fromchars.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../utils/startswith.hpp"

namespace WiThrottle {

struct Address
{
  uint16_t address;
  bool isLong;
  bool isWildcard;

  constexpr bool operator !=(const Address& other) const noexcept
  {
    return
      (address != other.address) ||
      (isLong != other.isLong) ||
      (isWildcard != other.isWildcard);
  }
};

static bool parseAddress(std::string_view& sv, Address& address)
{
  if(sv.empty() || (sv[0] != 'S' && sv[0] != 'L' && sv[0] != '*'))
    return false;

  if(sv[0] == '*')
  {
    address.address = 0;
    address.isLong = false;
    address.isWildcard = true;
    sv = sv.substr(1);
    return true;
  }

  auto r = fromChars(sv.substr(1), address.address);
  if(r.ec != std::errc())
    return false;

  address.isLong = (sv[0] == 'L');
  address.isWildcard = false;
  sv = sv.substr(r.ptr - sv.data());

  return true;
}

static std::string buildName(std::string name, char multiThrottleId)
{
  if(multiThrottleId != Kernel::invalidMultiThrottleId)
  {
    name.append(" [");
    name.push_back(multiThrottleId);
    name.push_back(']');
  }
  return name;
}

Kernel::Kernel(const Config& config)
  : m_config{config}
{
  assert(isEventLoopThread());
}

void Kernel::setConfig(const Config& config)
{
  m_ioContext.post(
    [this, newConfig=config]()
    {
      m_config = newConfig;
    });
}

void Kernel::setClock(std::shared_ptr<Clock> clock)
{
  assert(isEventLoopThread());
  assert(!m_running);
  assert(clock);
  m_clock = std::move(clock);
}

void Kernel::start()
{
  assert(isEventLoopThread());
  assert(m_ioHandler);
  assert(!m_running);

  m_running = true;

  m_powerOn = TriState::Undefined;

  m_thread = std::thread(
    [this]()
    {
      setThreadName("withrottle");
      auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
      m_ioContext.run();
    });

  if(m_clock)
    m_clockChangeConnection = m_clock->onChange.connect(
      [this](Clock::ClockEvent event, uint8_t multiplier, Time time)
      {
        postSendToAll(fastClock((time.hour() * 60U + time.minute()) * 60U, (event == Clock::ClockEvent::Freeze) ? 0 : multiplier));
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
}

void Kernel::stop()
{
  assert(isEventLoopThread());

  m_running = false;

  m_clockChangeConnection.disconnect();

  // remove all clients:
  auto itClient = m_clients.begin();
  while(itClient != m_clients.end())
  {
    auto& multiThrottles = itClient->second.multiThrottles;
    auto itMultiThrottle = multiThrottles.begin();
    while(itMultiThrottle != multiThrottles.end())
    {
      itMultiThrottle->second.address = 0; // don't try send a release to the client anymore
      itMultiThrottle->second.throttle->destroy();
      itMultiThrottle = multiThrottles.erase(itMultiThrottle);
    }
    itClient = m_clients.erase(itClient);
  }

  // stop iohandler and kernel thread:
  m_ioContext.post(
    [this]()
    {
      m_ioHandler->stop();
      m_ioContext.stop();
    });

  m_thread.join();
}

void Kernel::setPowerOn(bool on)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, on]()
    {
      if(m_powerOn != toTriState(on))
      {
        sendToAll(trackPower(on));
        m_powerOn = toTriState(on);
      }
    });
}

void Kernel::newClient(IOHandler::ClientId clientId)
{
  assert(isKernelThread());
  assert(m_running);

  sendTo(protocolVersion(), clientId);
  sendTo(serverType(), clientId);
  sendTo(serverVersion(), clientId);
  sendTo(rosterList({}), clientId);
  sendTo(trackPower(m_powerOn), clientId);

  EventLoop::call(
    [this, clientId]()
    {
      postSendTo(fastClock((m_clock->hour * 60U + m_clock->minute) * 60U, m_clock->running ? m_clock->multiplier : 0), clientId);

      if(auto* interface = dynamic_cast<Interface*>(m_throttleController))
      {
        auto throttle = HardwareThrottle::create(std::dynamic_pointer_cast<ThrottleController>(interface->shared_ptr<Interface>()), interface->world());
        auto [it, success] = m_clients.emplace(clientId, Client());
        assert(success);
        it->second.multiThrottles.emplace(invalidMultiThrottleId, MultiThrottle{0, false, std::move(throttle)});
      }
    });
}

void Kernel::clientGone(IOHandler::ClientId clientId)
{
  assert(isKernelThread());

  if(!m_running)
    return;

  EventLoop::call(
    [this, clientId]()
    {
      if(auto itClient = m_clients.find(clientId); itClient != m_clients.end())
      {
        for(auto& itMultiThrottle : itClient->second.multiThrottles)
        {
          const auto& throttle = itMultiThrottle.second.throttle;
          if(m_throttleController->removeThrottle(*throttle))
            throttle->destroy();
          else
            assert(false);
        }
        m_clients.erase(itClient);
      }
    });
}

void Kernel::receiveFrom(std::string_view message, IOHandler::ClientId clientId)
{
  assert(isKernelThread());
  assert(m_running);

  if(m_config.debugLogRXTX)
    EventLoop::call(
      [this, clientId, msg=std::string(message)]()
      {
        Log::log(m_logId, LogMessage::D2005_X_RX_X, clientId, msg);
      });

  if(message[0] == 'M') // Multi throttle command
  {
    if(message.size() >= 3)
    {
      static constexpr std::string_view seperator{"<;>"};

      [[maybe_unused]] const char multiThrottleId = message[1];
      const char command = message[2];
      message = message.substr(3);

      // address
      Address address;
      if(!parseAddress(message, address))
        return;

      // seperator
      if(!startsWith(message, seperator))
        return;
      message = message.substr(seperator.size());

      switch(command)
      {
        case 'A': // action
        {
          const auto throttleCommand = static_cast<ThrottleCommand>(message[0]);
          multiThrottleAction(clientId, multiThrottleId, address, throttleCommand, message.substr(1));
          break;
        }
        case '+': // add locomotive
        case 'S': // steal locomotive
        {
          if(address.isWildcard)
            return;

          Address addressRepeat;
          if(!parseAddress(message, addressRepeat) || !message.empty() || address != addressRepeat)
            return;

          EventLoop::call(
            [this, clientId, multiThrottleId, address, steal=(command == 'S')]()
            {
              const auto& throttle = getThottle(clientId, multiThrottleId);
              if(!throttle)
                return;

              switch(throttle->acquire(DecoderProtocol::DCC, address.address, address.isLong, steal))
              {
                case Throttle::AcquireResult::Success:
                {
                  if(auto* multiThrottle = getMultiThrottle(clientId, multiThrottleId))
                  {
                    multiThrottle->address = address.address;
                    multiThrottle->isLongAddress = address.isLong;
                  }
                  else
                    assert(false);

                  postSendTo(throttleCommand(multiThrottleId, '+', address.address, address.isLong), clientId);

                  std::unordered_map<uint32_t, std::string_view> functionNames;
                  for(const auto& f : *throttle->functions)
                    functionNames.emplace(f->number.value(), f->name.value());
                  postSendTo(throttleFuctionNames(multiThrottleId, address.address, address.isLong, functionNames), clientId);

                  for(const auto& f : *throttle->functions)
                    postSendTo(throttleFunction(multiThrottleId, address.address, address.isLong, f->number, f->value), clientId);

                  if(throttle->emergencyStop)
                    postSendTo(throttleEstop(multiThrottleId, address.address, address.isLong), clientId);
                  else
                    postSendTo(throttleSpeed(multiThrottleId, address.address, address.isLong, std::round(throttle->throttle * speedMax)), clientId);

                  postSendTo(throttleDirection(multiThrottleId, address.address, address.isLong, throttle->direction), clientId);

                  postSendTo(throttleSpeedStepMode(multiThrottleId, address.address, address.isLong, 128), clientId);
                  break;
                }
                case Throttle::AcquireResult::FailedNonExisting:
                  postSendTo(alert(std::string("Unknown ").append(address.isLong ? "long" : "short").append(" address: ").append(std::to_string(address.address))), clientId);
                  break;

                case Throttle::AcquireResult::FailedInUse:
                  postSendTo(throttleSteal(multiThrottleId, address.address, address.isLong), clientId);
                  break;
              }
            });

          break;
        }
        case '-':
        {
          EventLoop::call(
            [this, clientId, multiThrottleId, address]()
            {
              auto* multiThrottle = getMultiThrottle(clientId, multiThrottleId);
              if(multiThrottle && (address.isWildcard || (address.address == multiThrottle->address && address.isLong == multiThrottle->isLongAddress)))
              {
                multiThrottle->address = 0; // set address to zero so the release callback doesn't sent a release to.
                assert(multiThrottle->throttle);
                multiThrottle->throttle->release(false);

                // confirm release:
                // - WiThrottle needs this, else it won't release it.
                postSendTo(throttleRelease(multiThrottleId, address.address, address.isLong), clientId);
              }
            });
          break;
        }
      }
    }
  }
  else if(message == trackPowerOff() || message == trackPowerOn())
  {
    sendTo(alert("Track power control isn't allowed."), clientId);
    sendTo(trackPower(m_powerOn), clientId); // notify about current state
  }
  else if(message[0] == 'N') // throttle name
  {
    EventLoop::call(
      [this, clientId, name=std::string(message.substr(1))]()
      {
        if(auto itClient = m_clients.find(clientId); itClient != m_clients.end())
        {
          itClient->second.name = name;
          for(auto& itMultiThrottle : itClient->second.multiThrottles)
            itMultiThrottle.second.throttle->name.setValueInternal(buildName(name, itMultiThrottle.first));
        }
      });
  }
  else if(startsWith(message, "HU")) // throttle id
  {
    EventLoop::call(
      [this, clientId, id=std::string(message.substr(2))]()
      {
        if(auto itClient = m_clients.find(clientId); itClient != m_clients.end())
        {
          itClient->second.id = id;
        }
      });
  }
  else if(message == quit())
  {
    // post disconnect to finish current callback
    m_ioContext.post(
      [this, clientId]()
      {
        m_ioHandler->disconnect(clientId);
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

void Kernel::sendTo(std::string_view message, IOHandler::ClientId clientId)
{
  assert(isKernelThread());

  if(m_ioHandler->sendTo(message, clientId))
  {
    if(m_config.debugLogRXTX)
      EventLoop::call(
        [this, clientId, msg=std::string(message)]()
        {
          Log::log(m_logId, LogMessage::D2004_X_TX_X, clientId, msg);
        });
  }
}

void Kernel::sendToAll(std::string_view message)
{
  assert(isKernelThread());

  if(m_ioHandler->hasClients() && m_ioHandler->sendToAll(message))
  {
    if(m_config.debugLogRXTX)
      EventLoop::call(
        [this, msg=std::string(message)]()
        {
          Log::log(m_logId, LogMessage::D2001_TX_X, msg);
        });
  }
}

Kernel::MultiThrottle* Kernel::getMultiThrottle(IOHandler::ClientId clientId, char multiThrottleId)
{
  assert(isEventLoopThread());

  if(auto itClient = m_clients.find(clientId); itClient != m_clients.end())
  {
    auto& multiThrottles = itClient->second.multiThrottles;
    if(auto itMultiThrottle = multiThrottles.find(multiThrottleId); itMultiThrottle != multiThrottles.end())
      return &itMultiThrottle->second;
  }

  return nullptr;
}

const std::shared_ptr<HardwareThrottle>& Kernel::getThottle(IOHandler::ClientId clientId, char multiThrottleId)
{
  assert(isEventLoopThread());

  static const std::shared_ptr<HardwareThrottle> noThrottle;

  if(auto itClient = m_clients.find(clientId); itClient != m_clients.end())
  {
    auto& multiThrottles = itClient->second.multiThrottles;

    // check for multi throttle id:
    auto itMultiThrottle = multiThrottles.find(multiThrottleId);
    if(itMultiThrottle != multiThrottles.end())
      return itMultiThrottle->second.throttle;

    // check for invalid multi throttle id (multi throttle id is unknown until first throttle command):
    itMultiThrottle = multiThrottles.find(invalidMultiThrottleId);
    if(itMultiThrottle != multiThrottles.end())
    {
      auto n = multiThrottles.extract(invalidMultiThrottleId);
      n.key() = multiThrottleId;
      auto it = multiThrottles.insert(std::move(n)).position;
      const auto& throttle = it->second.throttle;
      throttle->name.setValueInternal(buildName(itClient->second.name, multiThrottleId));
      throttle->released.connect(std::bind(&Kernel::throttleReleased, this, clientId, multiThrottleId));
      return throttle;
    }

    // create a new throttle:
    if(auto* interface = dynamic_cast<Interface*>(m_throttleController))
    {
      auto newThrottle = HardwareThrottle::create(std::dynamic_pointer_cast<ThrottleController>(interface->shared_ptr<Interface>()), interface->world());
      auto [it, success] = multiThrottles.emplace(multiThrottleId, MultiThrottle{0, false, std::move(newThrottle)});
      assert(success);
      const auto& throttle = it->second.throttle;
      throttle->name.setValueInternal(buildName(itClient->second.name, multiThrottleId));
      throttle->released.connect(std::bind(&Kernel::throttleReleased, this, clientId, multiThrottleId));
      return throttle;
    }
  }

  return noThrottle;
}

void Kernel::multiThrottleAction(IOHandler::ClientId clientId, char multiThrottleId, const Address& /*address*/, ThrottleCommand throttleCommand, std::string_view message)
{
  assert(isKernelThread());

  switch(throttleCommand)
  {
    case ThrottleCommand::SetSpeed:
    {
      int value;
      auto r = fromChars(message, value);
      if(r.ec == std::errc() && value <= speedMax)
      {
        EventLoop::call(
          [this, clientId, multiThrottleId, value]()
          {
            if(const auto& throttle = getThottle(clientId, multiThrottleId); throttle && throttle->acquired())
            {
              if(value < 0)
              {
                throttle->emergencyStop = true;
              }
              else
              {
                throttle->emergencyStop = false;
                throttle->throttle = static_cast<float>(value) / speedMax;
              }
            }
          });
      }
      break;
    }
    case ThrottleCommand::SetDirection:
    {
      int value;
      auto r = fromChars(message, value);
      if(r.ec == std::errc())
      {
        EventLoop::call(
          [this, clientId, multiThrottleId, value]()
          {
            if(const auto& throttle = getThottle(clientId, multiThrottleId); throttle && throttle->acquired())
            {
              throttle->direction = (value == 0) ? Direction::Reverse : Direction::Forward;
            }
          });
      }
      break;
    }
    case ThrottleCommand::Function:
    case ThrottleCommand::ForceFunction:
    {
      const bool value = message[0] != '0';
      uint32_t number;
      auto r = fromChars(message.substr(1), number);
      if(r.ec == std::errc())
      {
        EventLoop::call(
          [this, clientId, multiThrottleId, number, force=(throttleCommand == ThrottleCommand::ForceFunction), value]()
          {
            if(const auto& throttle = getThottle(clientId, multiThrottleId); throttle && throttle->acquired())
            {
              if(const auto& function = throttle->getFunction(number))
              {
                if(force)
                  function->value = value;
                else if(value)
                  function->press();
                else
                  function->release();
              }
            }
          });
      }
      break;
    }
    case ThrottleCommand::Idle:
      EventLoop::call(
        [this, clientId, multiThrottleId]()
        {
          if(const auto& throttle = getThottle(clientId, multiThrottleId); throttle && throttle->acquired())
          {
            throttle->emergencyStop = false;
            throttle->throttle = Throttle::throttleStop;
          }
        });
      break;

    case ThrottleCommand::EmergencyStop:
      EventLoop::call(
        [this, clientId, multiThrottleId]()
        {
          if(const auto& throttle = getThottle(clientId, multiThrottleId); throttle && throttle->acquired())
          {
            throttle->emergencyStop = true;
          }
        });
      break;

    case ThrottleCommand::Query:
      if(message.size() == 1)
      {
        switch(message[0])
        {
          case 'V': // speed
            EventLoop::call(
              [this, clientId, multiThrottleId]()
              {
                if(const auto* multiThrottle = getMultiThrottle(clientId, multiThrottleId))
                {
                  if(multiThrottle->throttle->emergencyStop)
                    postSendTo(throttleEstop(multiThrottleId, multiThrottle->address, multiThrottle->isLongAddress), clientId);
                  else
                    postSendTo(throttleSpeed(multiThrottleId, multiThrottle->address, multiThrottle->isLongAddress, std::round(multiThrottle->throttle->throttle * speedMax)), clientId);
                }
              });
            break;

          case 'R': // direction
            EventLoop::call(
              [this, clientId, multiThrottleId]()
              {
                if(const auto* multiThrottle = getMultiThrottle(clientId, multiThrottleId))
                {
                  postSendTo(throttleDirection(multiThrottleId, multiThrottle->address, multiThrottle->isLongAddress, multiThrottle->throttle->direction), clientId);
                }
              });
            break;
        }
      }
      break;

    case ThrottleCommand::Consist:
    case ThrottleCommand::ConsistLeadFromRosterEntry:
    case ThrottleCommand::Dispatch:
    case ThrottleCommand::SetAddressFromRosterEntry:
    case ThrottleCommand::SetLongAddress:
    case ThrottleCommand::MomentaryFunction:
    case ThrottleCommand::Quit:
    case ThrottleCommand::Release:
    case ThrottleCommand::SetShortAddress:
    case ThrottleCommand::SetSpeedStepMode:
      // unsupported
      break;
  }
}

void Kernel::throttleReleased(IOHandler::ClientId clientId, char multiThrottleId)
{
  assert(isEventLoopThread());

  const auto* multiThrottle = getMultiThrottle(clientId, multiThrottleId);
  if(multiThrottle && multiThrottle->address != 0)
    postSendTo(throttleRelease(multiThrottleId, multiThrottle->address, multiThrottle->isLongAddress), clientId);
}

}
