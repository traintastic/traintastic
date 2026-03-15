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
#include "simulator/cbussimulator.hpp"
#include "../dcc/dcc.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../utils/inrange.hpp"
#include "../../../utils/setthreadname.hpp"

namespace {

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

    case OpCode::ASON:
    {
      const auto& ason = static_cast<const AccessoryShortOn&>(message);
      EventLoop::call(
        [this, eventNumber=ason.deviceNumber()]()
        {
          if(onShortEvent) [[likely]]
          {
            onShortEvent(eventNumber, true);
          }
        });
      break;
    }
    case OpCode::ASOF:
    {
      const auto& asof = static_cast<const AccessoryShortOff&>(message);
      EventLoop::call(
        [this, eventNumber=asof.deviceNumber()]()
        {
          if(onShortEvent) [[likely]]
          {
            onShortEvent(eventNumber, false);
          }
        });
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
      EventLoop::call(
        [this, nodeNumber=acon.nodeNumber(), eventNumber=acon.eventNumber()]()
        {
          if(onLongEvent) [[likely]]
          {
            onLongEvent(nodeNumber, eventNumber, false);
          }
        });
      break;
    }
    case OpCode::ACOF:
    {
      const auto& acof = static_cast<const AccessoryOff&>(message);
      EventLoop::call(
        [this, nodeNumber=acof.nodeNumber(), eventNumber=acof.eventNumber()]()
        {
          if(onLongEvent) [[likely]]
          {
            onLongEvent(nodeNumber, eventNumber, false);
          }
        });
      break;
    }
    case OpCode::PLOC:
    {
      const auto& ploc = static_cast<const EngineReport&>(message);
      const auto key = makeAddressKey(ploc.address(), ploc.isLongAddress());
      if(m_engineGLOCs.contains(key))
      {
        m_engineGLOCs.erase(key);

        if(auto it = m_engines.find(key); it != m_engines.end())
        {
          auto& engine = it->second;
          engine.session = ploc.session;

          sendSetEngineSessionMode(ploc.session, engine.speedSteps);
          sendSetEngineSpeedDirection(ploc.session, engine.speed, engine.directionForward);

          for(const auto& [number, value] : engine.functions)
          {
            sendSetEngineFunction(ploc.session, number, value);
          }
        }
        else // we're no longer in need of control (rare but possible)
        {
          send(ReleaseEngine(ploc.session));
        }
      }
      break;
    }
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

void Kernel::setEngineSpeedDirection(uint16_t address, bool longAddress, uint8_t speedStep, uint8_t speedSteps, bool eStop, bool directionForward)
{
  assert(isEventLoopThread());

  const uint8_t speed = eStop ? 1 : (speedStep > 0 ? speedStep + 1 : 0);

  m_ioContext.post(
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

  m_ioContext.post(
    [this, address, longAddress, number, value]()
    {
      auto& engine = m_engines[makeAddressKey(address, longAddress)];
      engine.functions[number] = value;
      if(engine.session) // we're in control
      {
        sendSetEngineFunction(*engine.session, number, value);
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

  m_ioContext.post(
    [this, deviceNumber, on]()
    {
      if(on)
      {
        send(AccessoryShortOn(Config::nodeId, deviceNumber));
      }
      else
      {
        send(AccessoryShortOff(Config::nodeId, deviceNumber));
      }
    });
}

void Kernel::setAccessory(uint16_t nodeNumber, uint16_t eventNumber, bool on)
{
  assert(isEventLoopThread());

  m_ioContext.post(
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

bool Kernel::send(std::vector<uint8_t> message)
{
  assert(isEventLoopThread());

  if(!inRange<size_t>(message.size(), 1, 8))
  {
    return false;
  }

  m_ioContext.post(
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

  m_ioContext.post(
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

  if(auto ec = m_ioHandler->send(message); ec)
  {
    (void)ec; // FIXME: handle error
  }
}

void Kernel::sendGetEngineSession(uint16_t address, bool longAddress)
{
  assert(isKernelThread());
  const auto key = makeAddressKey(address, longAddress);
  if(!m_engineGLOCs.contains(key))
  {
    m_engineGLOCs.emplace(key);
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
}

void Kernel::sendSetEngineFunction(uint8_t session, uint8_t number, bool value)
{
  assert(isKernelThread());
  if(value)
  {
    send(SetEngineFunctionOn(session, number));
  }
  else
  {
    send(SetEngineFunctionOff(session, number));
  }
}

}
