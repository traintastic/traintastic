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
#include "cbuscanmessageutils.hpp"
#include "cbustostring.hpp"
#include "iohub/cbusiohub.hpp"
#include "simulator/cbussimulator.hpp"
#include "../dcc/dcc.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../utils/inrange.hpp"
#include "../../../utils/setthreadname.hpp"

namespace {

using namespace std::chrono_literals;

static constexpr auto queryNodeNumberTimeout = 250ms;
static constexpr auto readNodeParameterTimeout = 50ms;
static constexpr auto requestShortEventTimeout = 50ms;
static constexpr auto requestLongEventTimeout = 50ms;
static constexpr auto requestCommandStationStatusTimeout = 100ms;

constexpr uint16_t makeAddressKey(uint16_t address, bool longAddress)
{
  return longAddress ? (0xC000 | (address & 0x3FFF)) : (address & 0x7F);
}

constexpr CBUS::SetEngineSessionMode::SpeedMode toSpeedMode(uint8_t speedSteps)
{
  using enum CBUS::SetEngineSessionMode::SpeedMode;

  switch(speedSteps)
  {
    case 14:
      return SpeedMode14;

    case 28:
      return SpeedMode28;

    default:
      return SpeedMode128;
  }
}

}

namespace CBUS {

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_initializationTimer{ioContext()}
  , m_config{config}
  , m_engineKeepAliveTimer{ioContext()}
  , m_dccAccessoryTimer{ioContext()}
{
  assert(isEventLoopThread());
}

void Kernel::setConfig(const Config& config)
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this, newConfig=config]()
    {
      m_config = newConfig;
    });
}

void Kernel::setRequestEventsDuringInitialize(std::vector<uint16_t> shortEvents, std::vector<std::pair<uint16_t,uint16_t>> longEvents)
{
  assert(isEventLoopThread());

  std::reverse(shortEvents.begin(), shortEvents.end());
  std::reverse(longEvents.begin(), longEvents.end());

  m_initializationRequestShortEvents = std::move(shortEvents);
  m_initializationRequestLongEvents = std::move(longEvents);
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

  boost::asio::post(m_ioContext,
    [this]()
    {
      try
      {
        m_ioHandler->onReceive =
          [this](const CAN::Message& canMessage)
          {
            receive(canMessage);
            if(m_hub)
            {
              m_hub->send(canMessage);
            }
          };

        m_ioHandler->start();

        if(m_config.hubEnabled)
        {
          m_hub = std::make_shared<IOHub>(m_ioContext, logId, m_config.hubLocalhostOnly, m_config.hubPort);
          m_hub->start(
            [this](const CAN::Message& message)
            {
              receive(message);
              (void)m_ioHandler->send(message); // FICME: add error handling
            });
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
    });
}

void Kernel::stop()
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this]()
    {
      if(m_hub)
      {
        m_hub->stop();
      }
      m_ioHandler->stop();

      m_ioContext.stop();
    });

  m_thread.join();
}

void Kernel::started()
{
  assert(isKernelThread());

  nextState();
}

void Kernel::receive(const CAN::Message& canMessage)
{
  assert(isKernelThread());

  const auto canId = getCanId(canMessage);
  const auto& message = asMessage(canMessage);

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

    case OpCode::KLOC:
      receiveKLOC(static_cast<const ReleaseEngine&>(message));
      break;

    case OpCode::RLOC:
    {
      const auto rloc = static_cast<const RequestEngineSession&>(message);
      receiveGLOC(rloc.address(), rloc.isLongAddress(), GetEngineSession::Mode::Request);
      break;
    }
    case OpCode::GLOC:
    {
      const auto gloc = static_cast<const GetEngineSession&>(message);
      receiveGLOC(gloc.address(), gloc.isLongAddress(), gloc.mode());
      break;
    }
    case OpCode::DSPD:
      receiveDSPD(static_cast<const SetEngineSpeedDirection&>(message));
      break;

    case OpCode::DFNON:
    case OpCode::DFNOF:
      receiveDFNOx(static_cast<const SetEngineFunction&>(message));
      break;

    case OpCode::DFUN:
      receiveDFUN(static_cast<const SetEngineFunctions&>(message));
      break;

    case OpCode::ASON:
      receiveShortEvent(static_cast<const AccessoryShortOn&>(message).deviceNumber(), true);
      break;

    case OpCode::ASOF:
      receiveShortEvent(static_cast<const AccessoryShortOff&>(message).deviceNumber(), false);
      break;

    case OpCode::ARSON:
    {
      const auto eventNumber = static_cast<const AccessoryShortResponseEventOff&>(message).deviceNumber();
      receiveShortEvent(eventNumber, true);
      if(m_state == State::RequestShortEvents &&
          m_initializationRequestShortEvents.back() == eventNumber)
      {
        m_initializationRequestShortEvents.pop_back();
        requestShortEvent();
      }
      break;
    }
    case OpCode::ARSOF:
    {
      const auto eventNumber = static_cast<const AccessoryShortResponseEventOff&>(message).deviceNumber();
      receiveShortEvent(eventNumber, false);
      if(m_state == State::RequestShortEvents &&
          m_initializationRequestShortEvents.back() == eventNumber)
      {
        m_initializationRequestShortEvents.pop_back();
        requestShortEvent();
      }
      break;
    }
    case OpCode::ERR:
    {
      switch(static_cast<const CommandStationErrorMessage&>(message).errorCode)
      {
        using enum DCCErr;

        case LocoStackFull:
        {
          const auto& err = static_cast<const CommandStationLocoStackFullError&>(message);
          const auto key = makeAddressKey(err.address(), err.isLongAddress());
          if(m_engineGLOCs.contains(key))
          {
            m_engineGLOCs.erase(key);
            // FIXME: log error
          }
          break;
        }
        case SessionCancelled:
          if(auto it = std::find_if(m_engines.begin(), m_engines.end(),
            [session=static_cast<const CommandStationSessionCancelled&>(message).session()](const auto& item)
            {
              return item.second.session && *item.second.session == session;
            }); it != m_engines.end())
          {
            it->second.session = std::nullopt;

            if(m_engineKeepAliveTimerActive && m_engineKeepAliveSession == *it->second.session)
            {
              restartEngineKeepAliveTimer();
            }

            EventLoop::call(
              [this, key=it->first]()
              {
                if(onEngineSessionCancelled) [[likely]]
                {
                  onEngineSessionCancelled(key & 0x3FFF, (key & 0xC000) == 0xC000);
                }
              });
          }
          break;

        case LocoAddressTaken:
        case SessionNotPresent:
        case ConsistEmpty:
        case LocoNotFound:
        case CANBusError:
        case InvalidRequest:
          break;
      }
      break;
    }
    case OpCode::ACON:
    {
      const auto& acon = static_cast<const AccessoryOn&>(message);
      receiveLongEvent(acon.nodeNumber(), acon.eventNumber(), true);
      break;
    }
    case OpCode::ACOF:
    {
      const auto& acof = static_cast<const AccessoryOff&>(message);
      receiveLongEvent(acof.nodeNumber(), acof.eventNumber(), false);
      break;
    }
    case OpCode::ARON:
    {
      const auto& aron = static_cast<const AccessoryResponseEventOn&>(message);
      receiveLongEvent(aron.nodeNumber(), aron.eventNumber(), true);
      if(m_state == State::RequestShortEvents &&
          m_initializationRequestLongEvents.back().first == aron.nodeNumber() &&
          m_initializationRequestLongEvents.back().second == aron.eventNumber())
      {
        m_initializationRequestLongEvents.pop_back();
        requestLongEvent();
      }
      break;
    }
    case OpCode::AROF:
    {
      const auto& arof = static_cast<const AccessoryResponseEventOff&>(message);
      receiveLongEvent(arof.nodeNumber(), arof.eventNumber(), false);
      if(m_state == State::RequestShortEvents &&
          m_initializationRequestLongEvents.back().first == arof.nodeNumber() &&
          m_initializationRequestLongEvents.back().second == arof.eventNumber())
      {
        m_initializationRequestLongEvents.pop_back();
        requestLongEvent();
      }
      break;
    }
    case OpCode::PARAN:
      if(m_state == State::ReadNodeParameters && !m_readNodeParameters.empty())
      {
        const auto& rqnpn = m_readNodeParameters.front();
        const auto& paran = static_cast<const NodeParameterResponse&>(message);

        if(rqnpn.nodeNumber() == paran.nodeNumber() && rqnpn.parameter == paran.parameter)
        {
          m_readNodeParameters.pop();
          m_initializationTimer.cancel();

          EventLoop::call(
            [this, canId, paran]()
            {
              if(onNodeParameterResponse) [[likely]]
              {
                onNodeParameterResponse(canId, paran.nodeNumber(), paran.parameter, paran.value);
              }
            });

          readNodeParameter();
        }
      }
      break;

    case OpCode::PNN:
      if(m_state == State::QueryNodes)
      {
        restartInitializationTimer(queryNodeNumberTimeout);

        const auto& pnn = static_cast<const PresenceOfNode&>(message);

        m_readNodeParameters.emplace(ReadNodeParameter(pnn.nodeNumber(), NodeParameter::VersionMajor));
        m_readNodeParameters.emplace(ReadNodeParameter(pnn.nodeNumber(), NodeParameter::VersionMinor));
        m_readNodeParameters.emplace(ReadNodeParameter(pnn.nodeNumber(), NodeParameter::BetaReleaseCode));

        EventLoop::call(
          [this, canId, pnn]()
          {
            if(onPresenceOfNode) [[likely]]
            {
              onPresenceOfNode(canId, pnn.nodeNumber(), pnn.manufacturerId, pnn.moduleId, pnn.flimMode(), pnn.supportsServiceDiscovery());
            }
          });
      }
      break;

    case OpCode::PLOC:
    {
      const auto& ploc = static_cast<const EngineReport&>(message);
      const auto key = makeAddressKey(ploc.address(), ploc.isLongAddress());
      auto gloc = m_engineGLOCs.find(key);
      const auto owner = (gloc != m_engineGLOCs.end()) ? gloc->second : Owner::CBUS;

      EventLoop::call(
        [this, ploc, owner]()
        {
          if(onEngineSessionAcquire) [[likely]]
          {
            onEngineSessionAcquire(ploc.session, owner != Owner::Traintastic, ploc.address(), ploc.isLongAddress());
          }
        });

      Engine* engine = nullptr;
      if(auto it = m_engines.find(key); it != m_engines.end())
      {
        engine = &it->second;
      }
      else if(owner == Owner::CBUS) // existing session we didn't know about
      {
        engine = &m_engines[makeAddressKey(ploc.address(), ploc.isLongAddress())];
      }
      else if(owner == Owner::Traintastic) // we're no longer in need of control (rare but possible)
      {
        send(ReleaseEngine(ploc.session));
      }

      if(engine)
      {
        engine->session = ploc.session;
        engine->owner = owner;

        if(engine->owner == Owner::Traintastic)
        {
          sendSetEngineSessionMode(ploc.session, engine->speedSteps);
          sendSetEngineSpeedDirection(ploc.session, engine->speed, engine->directionForward);

          for(const auto& [number, value] : engine->functions)
          {
            sendSetEngineFunction(ploc.session, number, value);
          }

          engine->lastCommand = std::chrono::steady_clock::now();

          if(!m_engineKeepAliveTimerActive)
          {
            restartEngineKeepAliveTimer();
          }
        }
        else if(engine->owner == Owner::CBUS)
        {
          engine->speed = ploc.speed();
          engine->directionForward = ploc.directionForward();

          EventLoop::call(
            [this, session=*engine->session, speed=engine->speed, directionForward=engine->directionForward]()
            {
              if(onEngineSpeedDirectionChanged) [[likely]]
              {
                onEngineSpeedDirectionChanged(session, speed, directionForward);
              }
            });

          for(uint8_t fn : ploc.numbers())
          {
            engine->functions[fn] = ploc.f(fn);
            EventLoop::call(
              [this, session=*engine->session, number=fn, value=engine->functions[fn]]()
              {
                if(onEngineFunctionChanged) [[likely]]
                {
                  onEngineFunctionChanged(session, number, value);
                }
              });
          }
        }
      }

      if(gloc != m_engineGLOCs.end())
      {
        m_engineGLOCs.erase(gloc);
      }
      break;
    }
    case OpCode::STAT:
      if(m_state == State::GetCommandStationStatus)
      {
        m_initializationTimer.cancel();
        nextState();
      }
      break;

    default:
      break;
  }

  // external listeners:
  for(const auto& [handle, opCode] : m_onReceiveFilters)
  {
    if(message.opCode == opCode)
    {
      auto buffer = std::make_shared<std::byte[]>(message.size());
      std::memcpy(buffer.get(), &message, message.size());
      EventLoop::call(
        [this, handle, canId, data=std::move(buffer)]()
        {
          // check if still registered, could theoretically be unregister after filtering and before callback:
          if(auto it = m_onReceiveCallbacks.find(handle); it != m_onReceiveCallbacks.end())
          {
            it->second(canId, *reinterpret_cast<const Message*>(data.get()));
          }
        });
    }
  }
}

size_t Kernel::registerOnReceive(OpCode opCode, std::function<void(uint8_t, const Message&)> callback)
{
  assert(isEventLoopThread());

  while(++m_onReceiveHandle == 0);

  m_onReceiveCallbacks.emplace(m_onReceiveHandle, std::move(callback));

  m_ioContext.post(
    [this, handle=m_onReceiveHandle, opCode]()
    {
      m_onReceiveFilters.emplace(handle, opCode);
    });

  return m_onReceiveHandle;
}

void Kernel::unregisterOnReceive(size_t handle)
{
  assert(isEventLoopThread());

  m_onReceiveCallbacks.erase(handle);

  m_ioContext.post(
    [this, handle]()
    {
      m_onReceiveFilters.erase(handle);
    });
}

void Kernel::trackOff()
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
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

  boost::asio::post(m_ioContext,
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

  boost::asio::post(m_ioContext,
    [this]()
    {
      send(RequestEmergencyStop());
    });
}

void Kernel::queryEngine(uint8_t session)
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this, session]()
    {
      send(QueryEngine(session));
    });
}

void Kernel::setEngineSpeedDirection(uint16_t address, bool longAddress, uint8_t speedStep, uint8_t speedSteps, bool eStop, bool directionForward)
{
  assert(isEventLoopThread());

  const uint8_t speed = eStop ? 1 : (speedStep > 0 ? speedStep + 1 : 0);

  boost::asio::post(m_ioContext,
    [this, address, longAddress, speed, speedSteps, directionForward]()
    {
      auto& engine = m_engines[makeAddressKey(address, longAddress)];
      const bool speedStepsChanged = engine.speedSteps != speedSteps;
      engine.speedSteps = speedSteps;
      engine.speed = speed;
      engine.directionForward = directionForward;

      if(engine.session) // we're in control
      {
        if(speedStepsChanged)
        {
          sendSetEngineSessionMode(*engine.session, engine.speedSteps);
        }
        sendSetEngineSpeedDirection(*engine.session, engine.speed, engine.directionForward);

        engine.lastCommand = std::chrono::steady_clock::now();

        if(!m_engineKeepAliveTimerActive || (m_engineKeepAliveTimerActive && m_engineKeepAliveSession == *engine.session))
        {
          restartEngineKeepAliveTimer();
        }
      }
      else // take control
      {
        sendGetEngineSession(address, longAddress);
      }
    });
}

void Kernel::setEngineFunction(uint16_t address, bool longAddress, uint8_t number, bool value)
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this, address, longAddress, number, value]()
    {
      auto& engine = m_engines[makeAddressKey(address, longAddress)];
      engine.functions[number] = value;
      if(engine.session) // we're in control
      {
        sendSetEngineFunction(*engine.session, number, value);

        engine.lastCommand = std::chrono::steady_clock::now();

        if(!m_engineKeepAliveTimerActive || (m_engineKeepAliveTimerActive && m_engineKeepAliveSession == *engine.session))
        {
          restartEngineKeepAliveTimer();
        }
      }
      else // take control
      {
        sendGetEngineSession(address, longAddress);
      }
    });
}

void Kernel::setAccessoryShort(uint16_t deviceNumber, bool on)
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this, deviceNumber, on]()
    {
      if(on)
      {
        send(AccessoryShortOn(m_config.shortEventNodeNumber, deviceNumber));
      }
      else
      {
        send(AccessoryShortOff(m_config.shortEventNodeNumber, deviceNumber));
      }
    });
}

void Kernel::setAccessory(uint16_t nodeNumber, uint16_t eventNumber, bool on)
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this, nodeNumber, eventNumber, on]()
    {
      if(on)
      {
        send(AccessoryOn(nodeNumber, eventNumber));
      }
      else
      {
        send(AccessoryOff(nodeNumber, eventNumber));
      }
    });
}

void Kernel::setDccAccessory(uint16_t address, bool secondOutput)
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this, address, secondOutput]()
    {
      send(RequestDCCPacket<sizeof(DCC::SetSimpleAccessory) + 1>(DCC::SetSimpleAccessory(address, secondOutput, true), Config::dccAccessoryRepeat));
      const bool wasEmpty = m_dccAccessoryQueue.empty();
      m_dccAccessoryQueue.emplace(std::make_pair(
        std::chrono::steady_clock::now() + m_config.dccAccessorySwitchTime,
        DCC::SetSimpleAccessory(address, secondOutput, false)
      ));
      if(wasEmpty)
      {
        startDccAccessoryTimer();
      }
    });
}

void Kernel::setDccAdvancedAccessoryValue(uint16_t address, uint8_t aspect)
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this, address, aspect]()
    {
      send(RequestDCCPacket<sizeof(DCC::SetAdvancedAccessoryValue) + 1>(DCC::SetAdvancedAccessoryValue(address, aspect), Config::dccExtRepeat));
    });
}

bool Kernel::send(std::vector<uint8_t> message)
{
  assert(isEventLoopThread());

  if(!inRange<size_t>(message.size(), 1, 8))
  {
    return false;
  }

  boost::asio::post(m_ioContext,
    [this, msg=std::move(message)]()
    {
      send(*reinterpret_cast<const Message*>(msg.data()));
    });

  return true;
}

bool Kernel::sendDCC(std::vector<uint8_t> dccPacket, uint8_t repeat)
{
  assert(isEventLoopThread());

  if(!inRange<size_t>(dccPacket.size(), 2, 5) || repeat == 0)
  {
    return false;
  }

  dccPacket.emplace_back(DCC::calcChecksum(dccPacket));

  boost::asio::post(m_ioContext,
    [this, packet=std::move(dccPacket), repeat]()
    {
      switch(packet.size())
      {
        case 3:
          send(RequestDCCPacket<3>(packet, repeat));
          break;

        case 4:
          send(RequestDCCPacket<4>(packet, repeat));
          break;

        case 5:
          send(RequestDCCPacket<5>(packet, repeat));
          break;

        case 6:
          send(RequestDCCPacket<6>(packet, repeat));
          break;

        default: [[unlikely]]
          assert(false);
          break;
      }
    });

  return true;
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

  const auto canMessage = toCANMessage(message, m_config.canId);

  if(auto ec = m_ioHandler->send(canMessage); ec)
  {
    (void)ec; // FIXME: handle error
  }

  if(m_hub)
  {
    m_hub->send(canMessage);
  }
}

void Kernel::sendGetEngineSession(uint16_t address, bool longAddress)
{
  assert(isKernelThread());
  const auto key = makeAddressKey(address, longAddress);
  if(!m_engineGLOCs.contains(key))
  {
    m_engineGLOCs.emplace(key, Owner::Traintastic);
    send(GetEngineSession(address, longAddress, GetEngineSession::Mode::Steal));
  }
}

void Kernel::sendSetEngineSessionMode(uint8_t session, uint8_t speedSteps)
{
  assert(isKernelThread());
  // FIXME: what to do with: serviceMode and soundControlMode?
  send(SetEngineSessionMode(session, toSpeedMode(speedSteps), false, false));
}

void Kernel::sendSetEngineSpeedDirection(uint8_t session, uint8_t speed, bool directionForward)
{
  assert(isKernelThread());

  send(SetEngineSpeedDirection(session, speed, directionForward));

  EventLoop::call(
    [this, session, speed, directionForward]()
    {
      if(onEngineSpeedDirectionChanged) [[likely]]
      {
        onEngineSpeedDirectionChanged(session, speed, directionForward);
      }
    });
}

void Kernel::sendSetEngineFunction(uint8_t session, uint8_t number, bool value)
{
  assert(isKernelThread());

  send(SetEngineFunction(session, number, value));

  EventLoop::call(
    [this, session, number, value]()
    {
      if(onEngineFunctionChanged) [[likely]]
      {
        onEngineFunctionChanged(session, number, value);
      }
    });
}

void Kernel::receiveGLOC(uint16_t address, bool longAddress, GetEngineSession::Mode /*mode*/)
{
  assert(isKernelThread());
  const auto key = makeAddressKey(address, longAddress);
  if(!m_engineGLOCs.contains(key))
  {
    m_engineGLOCs.emplace(key, Owner::CBUS);
    (void)m_engines[makeAddressKey(address, longAddress)]; // create entry if not exists
  }
}

void Kernel::receiveDFNOx(const SetEngineFunction& message)
{
  assert(isKernelThread());
  EventLoop::call(
    [this, message]()
    {
      if(onEngineFunctionChanged) [[likely]]
      {
        onEngineFunctionChanged(message.session, message.number, message.on());
      }
    });
}

void Kernel::receiveDFUN(const CBUS::SetEngineFunctions& message)
{
  assert(isKernelThread());
  EventLoop::call(
    [this, message]()
    {
      if(onEngineFunctionChanged) [[likely]]
      {
        switch(message.range)
        {
          using enum SetEngineFunctions::Range;

          case F0F4:
            for(auto fn : message.numbers())
            {
              onEngineFunctionChanged(message.session, fn, static_cast<const SetEngineFunctionsF0F4&>(message).f(fn));
            }
            break;

          case F5F8:
            for(auto fn : message.numbers())
            {
              onEngineFunctionChanged(message.session, fn, static_cast<const SetEngineFunctionsF5F8&>(message).f(fn));
            }
            break;

          case F9F12:
            for(auto fn : message.numbers())
            {
              onEngineFunctionChanged(message.session, fn, static_cast<const SetEngineFunctionsF9F12&>(message).f(fn));
            }
            break;

          case F13F20:
            for(auto fn : message.numbers())
            {
              onEngineFunctionChanged(message.session, fn, static_cast<const SetEngineFunctionsF13F20&>(message).f(fn));
            }
            break;

          case F21F28:
            for(auto fn : message.numbers())
            {
              onEngineFunctionChanged(message.session, fn, static_cast<const SetEngineFunctionsF21F28&>(message).f(fn));
            }
            break;
        }
      }
    });
}

void Kernel::receiveDSPD(const SetEngineSpeedDirection& message)
{
  assert(isKernelThread());
  EventLoop::call(
    [this, message]()
    {
      if(onEngineSpeedDirectionChanged) [[likely]]
      {
        onEngineSpeedDirectionChanged(message.session, message.speed(), message.directionForward());
      }
    });
}

void Kernel::receiveKLOC(const ReleaseEngine& message)
{
  assert(isKernelThread());
  EventLoop::call(
    [this, session=message.session]()
    {
      if(onEngineSessionReleased) [[likely]]
      {
        onEngineSessionReleased(session);
      }
    });
}

void Kernel::receiveShortEvent(uint16_t eventNumber, bool on)
{
  assert(isKernelThread());
  EventLoop::call(
    [this, eventNumber, on]()
    {
      if(onShortEvent) [[likely]]
      {
        onShortEvent(eventNumber, on);
      }
    });
}

void Kernel::receiveLongEvent(uint16_t nodeNumber, uint16_t eventNumber, bool on)
{
  assert(isKernelThread());
  EventLoop::call(
    [this, nodeNumber, eventNumber, on]()
    {
      if(onLongEvent) [[likely]]
      {
        onLongEvent(nodeNumber, eventNumber, on);
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
    case State::Initial: [[unlikely]]
      assert(false);
      break;

    case State::QueryNodes:
      send(QueryNodeNumber());
      restartInitializationTimer(queryNodeNumberTimeout);
      break;

    case State::ReadNodeParameters:
      readNodeParameter();
      break;

    case State::GetCommandStationStatus:
      send(RequestCommandStationStatus());
      restartInitializationTimer(requestCommandStationStatusTimeout);
      break;

    case State::RequestShortEvents:
      requestShortEvent();
      break;

    case State::RequestLongEvents:
      requestLongEvent();
      break;

    case State::Started:
      KernelBase::started();
      break;
  }
}

void Kernel::readNodeParameter()
{
  assert(m_state == State::ReadNodeParameters);
  if(m_readNodeParameters.empty())
  {
    nextState();
    return;
  }
  send(m_readNodeParameters.front());
  restartInitializationTimer(readNodeParameterTimeout);
}

void Kernel::requestShortEvent()
{
  assert(m_state == State::RequestShortEvents);
  if(m_initializationRequestShortEvents.empty())
  {
    nextState();
    return;
  }
  send(AccessoryShortRequestEvent(m_config.shortEventNodeNumber, m_initializationRequestShortEvents.back()));
  restartInitializationTimer(requestShortEventTimeout);
}

void Kernel::requestLongEvent()
{
  assert(m_state == State::RequestLongEvents);
  if(m_initializationRequestLongEvents.empty())
  {
    nextState();
    return;
  }
  send(AccessoryRequestEvent(m_initializationRequestLongEvents.back().first, m_initializationRequestLongEvents.back().second));
  restartInitializationTimer(requestLongEventTimeout);
}

void Kernel::restartInitializationTimer(std::chrono::milliseconds timeout)
{
  assert(isKernelThread());

  m_initializationTimer.cancel();

  m_initializationTimer.expires_after(timeout);
  m_initializationTimer.async_wait(
    [this](std::error_code ec)
    {
      if(ec)
      {
        return;
      }

      switch(m_state)
      {
        case State::QueryNodes:
          nextState();
          break;

        case State::ReadNodeParameters:
          m_readNodeParameters.pop();
          readNodeParameter();
          break;

        case State::GetCommandStationStatus:
          nextState();
          break;

        case State::RequestShortEvents:
          m_initializationRequestShortEvents.pop_back();
          requestShortEvent();
          break;

        case State::RequestLongEvents:
          m_initializationRequestLongEvents.pop_back();
          requestLongEvent();
          break;

        case State::Initial: [[unlikely]]
        case State::Started: [[unlikely]]
          assert(false);
          break;
      }
    });
}

void Kernel::restartEngineKeepAliveTimer()
{
  assert(isKernelThread());

  m_engineKeepAliveTimer.cancel();

  // find first expiring engine:
  std::chrono::steady_clock::time_point lastUpdate = std::chrono::steady_clock::time_point::max();
  for(const auto& [_, engine] : m_engines)
  {
    if(engine.session && engine.lastCommand < lastUpdate)
    {
      lastUpdate = engine.lastCommand;
      m_engineKeepAliveSession = *engine.session;
    }
  }

  m_engineKeepAliveTimerActive = (lastUpdate < std::chrono::steady_clock::time_point::max());

  if(m_engineKeepAliveTimerActive)
  {
    m_engineKeepAliveTimer.expires_at(lastUpdate + m_config.engineKeepAlive);
    m_engineKeepAliveTimer.async_wait(
      [this](std::error_code ec)
      {
        if(ec)
        {
          return;
        }

        send(SessionKeepAlive(m_engineKeepAliveSession));

        if(auto it = std::find_if(m_engines.begin(), m_engines.end(),
            [session=m_engineKeepAliveSession](const auto& item)
            {
              return item.second.session && *item.second.session == session;
            }); it != m_engines.end()) [[likely]]
        {
          it->second.lastCommand = std::chrono::steady_clock::now();
        }

        restartEngineKeepAliveTimer();
      });
  }
}

void Kernel::startDccAccessoryTimer()
{
  assert(isKernelThread());

  if(m_dccAccessoryQueue.empty()) [[unlikely]]
  {
    return;
  }

  m_dccAccessoryTimer.expires_at(m_dccAccessoryQueue.front().first);
  m_dccAccessoryTimer.async_wait(
    [this](std::error_code ec)
    {
      if(ec)
      {
        return;
      }

      send(RequestDCCPacket<sizeof(DCC::SetSimpleAccessory) + 1>(m_dccAccessoryQueue.front().second, Config::dccAccessoryRepeat));

      m_dccAccessoryQueue.pop();

      if(!m_dccAccessoryQueue.empty())
      {
        startDccAccessoryTimer();
      }
    });
}

}
