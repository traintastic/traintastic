/**
 * server/src/hardware/protocol/ecos/kernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2024 Reinder Feenstra
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
#include <algorithm>
#include <typeinfo>
#include "messages.hpp"
#include "simulation.hpp"
#include "object/ecos.hpp"
#include "object/locomotivemanager.hpp"
#include "object/locomotive.hpp"
#include "object/switchmanager.hpp"
#include "object/switch.hpp"
#include "object/feedbackmanager.hpp"
#include "object/feedback.hpp"
#include "../../protocol/dcc/dcc.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../input/inputcontroller.hpp"
#include "../../output/outputcontroller.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../utils/startswith.hpp"
#include "../../../utils/ltrim.hpp"
#include "../../../utils/rtrim.hpp"
#include "../../../utils/tohex.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"

#define ASSERT_IS_KERNEL_THREAD assert(isKernelThread())

namespace ECoS {

static constexpr DecoderProtocol toDecoderProtocol(LocomotiveProtocol locomotiveProtocol, uint16_t address)
{
  switch(locomotiveProtocol)
  {
    case LocomotiveProtocol::DCC14:
    case LocomotiveProtocol::DCC28:
    case LocomotiveProtocol::DCC128:
      return DCC::getProtocol(address);

    case LocomotiveProtocol::MM14:
    case LocomotiveProtocol::MM27:
    case LocomotiveProtocol::MM28:
      return DecoderProtocol::Motorola;

    case LocomotiveProtocol::SX32:
      return DecoderProtocol::Selectrix;

    case LocomotiveProtocol::Unknown:
    case LocomotiveProtocol::MMFKT:
      break;
  }
  return DecoderProtocol::None;
}

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
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
      m_config = newConfig;
    });
}

void Kernel::setOnEmergencyStop(std::function<void()> callback)
{
  assert(!m_started);
  m_onEmergencyStop = std::move(callback);
}

void Kernel::setOnGo(std::function<void()> callback)
{
  assert(!m_started);
  m_onGo = std::move(callback);
}

void Kernel::setOnObjectChanged(OnObjectChanged callback)
{
  assert(!m_started);
  m_onObjectChanged = std::move(callback);
}

void Kernel::setOnObjectRemoved(OnObjectRemoved callback)
{
  assert(!m_started);
  m_onObjectRemoved = std::move(callback);
}
void Kernel::setDecoderController(DecoderController* decoderController)
{
  assert(!m_started);
  m_decoderController = decoderController;
}

void Kernel::setInputController(InputController* inputController)
{
  assert(!m_started);
  m_inputController = inputController;
}

void Kernel::setOutputController(OutputController* outputController)
{
  assert(!m_started);
  m_outputController = outputController;
}

void Kernel::start()
{
  assert(m_ioHandler);
  assert(!m_started);
  assert(m_objects.empty());

  m_thread = std::thread(
    [this]()
    {
      setThreadName("ecos");
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

#ifndef NDEBUG
  m_started = true;
#endif
}

void Kernel::stop(Simulation* simulation)
{
  m_ioContext.post(
    [this]()
    {
      m_ioHandler->stop();
    });

  m_ioContext.stop();

  m_thread.join();

  if(simulation && !m_objects.empty()) // get simulation data
  {
    simulation->clear();

    // ECoS:
    {
      simulation->ecos.commandStationType = toString(ecos().model());
      simulation->ecos.protocolVersion = ::toString(ecos().protocolVersion());
      simulation->ecos.hardwareVersion = ::toString(ecos().hardwareVersion());
      simulation->ecos.applicationVersion = ::toString(ecos().applicationVersion());
      simulation->ecos.applicationVersionSuffix = ecos().applicationVersionSuffix();
      simulation->ecos.railcom = ecos().railcom();
      simulation->ecos.railcomPlus = ecos().railcomPlus();
    }

    // Locomotives / switches:
    for(const auto& it : m_objects)
    {
      if(const auto* locomotive = dynamic_cast<const Locomotive*>(it.second.get()))
      {
        simulation->locomotives.emplace_back(
          Simulation::Locomotive{
            {locomotive->id()},
            locomotive->protocol(),
            locomotive->address()});
      }
      else if(const auto* sw = dynamic_cast<const Switch*>(it.second.get()))
      {
        simulation->switches.emplace_back(
          Simulation::Switch{
            {sw->id()},
            sw->name1(),
            sw->name2(),
            sw->name3(),
            sw->address(),
            toString(sw->addrext()),
            std::string{toString(sw->type())},
            static_cast<int>(sw->symbol()),
            std::string{toString(sw->protocol())},
            sw->state(),
            std::string{toString(sw->mode())},
            sw->duration(),
            sw->variant()
            });
      }
    }

    // S88:
    {
      uint16_t id = ObjectId::s88;
      auto it = m_objects.find(id);
      while(it != m_objects.end())
      {
        if(const auto* feedback = dynamic_cast<const Feedback*>(it->second.get()))
          simulation->s88.emplace_back(Simulation::S88{{feedback->id()}, feedback->ports()});
        else
          break;
        it = m_objects.find(++id);
      }
    }
  }

  m_objects.clear();

#ifndef NDEBUG
  m_started = false;
#endif
}

void Kernel::started()
{
  assert(isKernelThread());

  m_objects.add(std::make_unique<ECoS>(*this));
  m_objects.add(std::make_unique<LocomotiveManager>(*this));
  m_objects.add(std::make_unique<SwitchManager>(*this));
  m_objects.add(std::make_unique<FeedbackManager>(*this));

  KernelBase::started();
}

void Kernel::receive(std::string_view message)
{
  if(m_config.debugLogRXTX)
  {
    std::string msg{rtrim(message, {'\r', '\n'})};
    std::replace_if(msg.begin(), msg.end(), [](char c){ return c == '\r' || c == '\n'; }, ';');
    EventLoop::call([this, msg](){ Log::log(logId, LogMessage::D2002_RX_X, msg); });
  }

  if(Reply reply; parseReply(message, reply))
  {
    auto it = m_objects.find(reply.objectId);
    if(it != m_objects.end())
      it->second->receiveReply(reply);
  }
  else if(Event event; parseEvent(message, event))
  {
    auto it = m_objects.find(event.objectId);
    if(it != m_objects.end())
      it->second->receiveEvent(event);
  }
  else
  {}//  EventLoop::call([this]() { Log::log(logId, LogMessage::E2018_ParseError); });
}

ECoS& Kernel::ecos()
{
  assert(isKernelThread() || m_ioContext.stopped());

  return static_cast<ECoS&>(*m_objects[ObjectId::ecos]);
}

void Kernel::ecosGoChanged(TriState value)
{
  ASSERT_IS_KERNEL_THREAD;

  if(value == TriState::False && m_onEmergencyStop)
    EventLoop::call([this]() { m_onEmergencyStop(); });
  else if(value == TriState::True && m_onGo)
    EventLoop::call([this]() { m_onGo(); });
}

Locomotive* Kernel::getLocomotive(DecoderProtocol protocol, uint16_t address, uint8_t speedSteps)
{
  ASSERT_IS_KERNEL_THREAD;

  //! \todo optimize this

  auto it = std::find_if(m_objects.begin(), m_objects.end(),
    [protocol, address, speedSteps](const auto& item)
    {
      auto* l = dynamic_cast<Locomotive*>(item.second.get());
      return
        l &&
        protocol == toDecoderProtocol(l->protocol(), l->address()) &&
        address == l->address()  &&
        speedSteps == l->speedSteps();
    });

  if(it != m_objects.end())
    return static_cast<Locomotive*>(it->second.get());

  return nullptr;
}

SwitchManager& Kernel::switchManager()
{
  ASSERT_IS_KERNEL_THREAD;

  return static_cast<SwitchManager&>(*m_objects[ObjectId::switchManager]);
}

void Kernel::emergencyStop()
{
  m_ioContext.post([this]() { ecos().stop(); });
}

void Kernel::go()
{
  m_ioContext.post([this]() { ecos().go(); });
}

void Kernel::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(has(changes, DecoderChangeFlags::Direction))
  {
    m_ioContext.post(
      [this,
        protocol=decoder.protocol.value(),
        address=decoder.address.value(),
        speedSteps=decoder.speedSteps.value(),
        direction=decoder.direction.value()]()
      {
        if(auto* locomotive = getLocomotive(protocol, address, speedSteps))
          locomotive->setDirection(direction);
      });
  }
  else if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Throttle))
  {
    m_ioContext.post(
      [this,
        protocol=decoder.protocol.value(),
        address=decoder.address.value(),
        speedSteps=decoder.speedSteps.value(),
        throttle=decoder.throttle.value(),
        emergencyStop=decoder.emergencyStop.value()]()
      {
        if(auto* locomotive = getLocomotive(protocol, address, speedSteps))
        {
          if(emergencyStop)
            locomotive->stop();
          else
            locomotive->setSpeedStep(Decoder::throttleToSpeedStep(throttle, locomotive->speedSteps()));
        }
      });
  }
  else if(has(changes, DecoderChangeFlags::FunctionValue) && functionNumber <= std::numeric_limits<uint8_t>::max())
  {
    m_ioContext.post(
      [this,
        protocol=decoder.protocol.value(),
        address=decoder.address.value(),
        speedSteps=decoder.speedSteps.value(),
        index=static_cast<uint8_t>(functionNumber),
        value=decoder.getFunctionValue(functionNumber)]()
      {
        if(auto* locomotive = getLocomotive(protocol, address, speedSteps))
          locomotive->setFunctionValue(index, value);
      });
  }
}

bool Kernel::setOutput(OutputChannel channel, uint32_t id, OutputValue value)
{
  assert(isEventLoopThread());

  switch(channel)
  {
    case OutputChannel::AccessoryDCC:
    case OutputChannel::AccessoryMotorola:
    {
      const auto switchProtocol = (channel == OutputChannel::AccessoryDCC) ? SwitchProtocol::DCC : SwitchProtocol::Motorola;
      m_ioContext.post(
        [this, switchProtocol, address=id, port=(std::get<OutputPairValue>(value) == OutputPairValue::Second)]()
        {
          switchManager().setSwitch(switchProtocol, address, port);
        });
      return true;
    }
    case OutputChannel::ECoSObject:
    {
      m_ioContext.post(
        [this, id, state=std::get<uint8_t>(value)]()
        {
          if(auto it = m_objects.find(id); it != m_objects.end())
          {
            if(auto* sw = dynamic_cast<Switch*>(it->second.get()))
            {
              sw->setState(state);
            }
          }
        });
      return true;
    }
    default: /*[[unlikely]]*/
      assert(false);
      break;
  }

  return false;
}

void Kernel::simulateInputChange(uint32_t channel, uint32_t address, SimulateInputAction action)
{
  if(!m_simulation)
    return;

  m_ioContext.post(
    [this, channel, address, action]()
    {
      switch(channel)
      {
        case InputChannel::s88:
        {
          uint16_t id = ObjectId::s88;
          uint32_t port = address - 1;
          auto it = m_objects.find(id);
          while(it != m_objects.end())
          {
            if(const auto* feedback = dynamic_cast<const Feedback*>(it->second.get()))
            {
              const auto ports = feedback->ports();
              if(port < ports)
              {
                uint16_t mask = 0;
                for(uint8_t i = 0; i < ports; i++)
                {
                  TriState value = feedback->operator[](i);
                  if(port == i)
                  {
                    switch(action)
                    {
                      case SimulateInputAction::SetFalse:
                        if(value == TriState::False)
                          return; // no change
                        value = TriState::False;
                        break;

                      case SimulateInputAction::SetTrue:
                        if(value == TriState::True)
                          return; // no change
                        value = TriState::True;
                        break;

                      case SimulateInputAction::Toggle:
                        value = (value == TriState::True) ? TriState::False : TriState::True;
                        break;
                    }
                  }
                  if(value == TriState::True)
                    mask |= 1 << i;
                }

                receive(
                  std::string("<EVENT ").append(std::to_string(feedback->id())).append(">\r\n")
                    .append(std::to_string(feedback->id())).append(" state[0x").append(mask != 0 ? ltrim(toHex(mask), '0') : std::string_view{"0"}).append("]>\r\n")
                    .append("<END 0 (OK)>\r\n"));

                break;
              }
              port -= ports;
            }
            it = m_objects.find(++id);
          }
          break;
        }
        case InputChannel::ecosDetector:
          //! \todo Implement ECoS detector simulation
          break;
      }
    });
}

void Kernel::switchManagerSwitched(SwitchProtocol protocol, uint16_t address, OutputPairValue value)
{
  ASSERT_IS_KERNEL_THREAD;

  if(!m_outputController)
    return;

  switch(protocol)
  {
    case SwitchProtocol::DCC:
      EventLoop::call(
        [this, address, value]()
        {
          m_outputController->updateOutputValue(OutputChannel::AccessoryDCC, address, value);
        });
      break;

    case SwitchProtocol::Motorola:
      EventLoop::call(
        [this, address, value]()
        {
          m_outputController->updateOutputValue(OutputChannel::AccessoryMotorola, address, value);
        });
      break;

    case SwitchProtocol::Unknown:
      assert(false);
      break;
  }
}

void Kernel::switchStateChanged(uint16_t objectId, uint8_t state)
{
  ASSERT_IS_KERNEL_THREAD;

  if(!m_outputController)
    return;

  EventLoop::call(
    [this, objectId, state]()
    {
      m_outputController->updateOutputValue(OutputChannel::ECoSObject, objectId, state);
    });
}

void Kernel::feedbackStateChanged(Feedback& object, uint8_t port, TriState value)
{
  if(!m_inputController)
    return;

  if(isS88FeedbackId(object.id()))
  {
    uint32_t offset = 1;
    for(uint16_t id = ObjectId::s88; id < object.id(); id++)
    {
      auto it = m_objects.find(id);
      if(it == m_objects.end())
      {
        assert(false);
        return;
      }

      const auto* feedback = dynamic_cast<const Feedback*>(it->second.get());
      if(!feedback)
      {
        assert(false);
        return;
      }

      offset += feedback->ports();
    }

    EventLoop::call(
      [this, address=offset + port, value]()
      {
        m_inputController->updateInputValue(InputChannel::s88, address, value);
      });
  }
  else // ECoS Detector
  {
    const uint16_t portsPerObject = 16;
    const uint16_t address = 1 + port + portsPerObject * (object.id() - ObjectId::ecosDetector);

    EventLoop::call(
      [this, address, value]()
      {
        m_inputController->updateInputValue(InputChannel::ecosDetector, address, value);
      });
  }
}

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

bool Kernel::objectExists(uint16_t objectId) const
{
  return m_objects.find(objectId) != m_objects.end();
}

void Kernel::addObject(std::unique_ptr<Object> object)
{
  objectChanged(*object);
  m_objects.add(std::move(object));
}

void Kernel::objectChanged(Object& object)
{
  if(!m_onObjectChanged) /*[[unlikely]]*/
  {
    return;
  }

  std::string objectName;
  if(auto* sw = dynamic_cast<const Switch*>(&object))
  {
    objectName = sw->nameUI();
  }

  EventLoop::call(
    [this, typeHash=typeid(object).hash_code(), objectId=object.id(), objectName]()
    {
      m_onObjectChanged(typeHash, objectId, objectName);
    });
}

void Kernel::removeObject(uint16_t objectId)
{
  m_objects.erase(objectId);
  if(m_onObjectRemoved) /*[[likely]]*/
  {
    m_onObjectRemoved(objectId);
  }
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

}
