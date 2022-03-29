/**
 * server/src/hardware/protocol/ecos/kernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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
#include "messages.hpp"
#include "object/ecos.hpp"
#include "object/locomotivemanager.hpp"
#include "object/switchmanager.hpp"
#include "object/feedbackmanager.hpp"
#include "object/feedback.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../input/inputcontroller.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../utils/startswith.hpp"
#include "../../../utils/rtrim.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"

namespace ECoS {

Kernel::Kernel(const Config& config)
  : m_ioContext{1}
  , m_decoderController{nullptr}
  , m_inputController{nullptr}
  , m_outputController{nullptr}
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
      m_config = newConfig;
    });
}

void Kernel::setOnStarted(std::function<void()> callback)
{
  assert(!m_started);
  m_onStarted = std::move(callback);
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
      m_ioHandler->start();

      m_objects.add(std::make_unique<ECoS>(*this));
      m_objects.add(std::make_unique<LocomotiveManager>(*this));
      m_objects.add(std::make_unique<SwitchManager>(*this));
      m_objects.add(std::make_unique<FeedbackManager>(*this));

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
  m_ioContext.post(
    [this]()
    {
      m_ioHandler->stop();
    });

  m_ioContext.stop();

  m_thread.join();

  m_objects.clear();

#ifndef NDEBUG
  m_started = false;
#endif
}

void Kernel::receive(std::string_view message)
{
  if(m_config.debugLogRXTX)
  {
    std::string msg{rtrim(message, {'\r', '\n'})};
    std::replace_if(msg.begin(), msg.end(), [](char c){ return c == '\r' || c == '\n'; }, ';');
    EventLoop::call([this, msg](){ Log::log(m_logId, LogMessage::D2002_RX_X, msg); });
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
  {}//  EventLoop::call([this]() { Log::log(m_logId, LogMessage::E2018_ParseError); });
}

ECoS& Kernel::ecos()
{
  return static_cast<ECoS&>(*m_objects[ObjectId::ecos]);
}

void Kernel::ecosGoChanged(TriState value)
{
  if(value == TriState::False && m_onEmergencyStop)
    EventLoop::call([this]() { m_onEmergencyStop(); });
  else if(value == TriState::True && m_onGo)
    EventLoop::call([this]() { m_onGo(); });
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
  (void)(decoder);
  (void)(changes);
  (void)(functionNumber);
}

bool Kernel::setOutput(uint16_t address, bool value)
{
  (void)(address);
  (void)(value);

  return false;
}

void Kernel::feedbackStateChanged(Feedback& object, uint8_t port, TriState value)
{
  if(!m_inputController)
    return;

  if(isS88FeedbackId(object.id()))
  {
    const uint16_t portsPerObject = 16;
    const uint16_t address = 1 + port + portsPerObject * (object.id() - ObjectId::s88);

    EventLoop::call(
      [this, address, value]()
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

}
