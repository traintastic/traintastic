/**
 * server/src/hardware/protocol/ecos/kernel.cpp
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
#include <algorithm>
#include "messages.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
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

  // reset all state values
  m_emergencyStop = TriState::Undefined;

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

      for(auto id : {ObjectId::ecos, ObjectId::locManager, ObjectId::switchManager, ObjectId::feedbackManager})
        send(request(id, {Option::view}));

      send(get(ObjectId::ecos, {Option::info}));

      // query some lists
      send(queryObjects(ObjectId::locManager, {Option::addr, Option::protocol}));
      send(queryObjects(ObjectId::switchManager, {Option::addr}));
      send(queryObjects(ObjectId::feedbackManager));

      // query an object
      send(request(1000, {Option::view}));
      send(get(1000, {Option::addr}));
      send(get(1000, {Option::name}));

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

#ifndef NDEBUG
  m_started = false;
#endif
}

void Kernel::receive(std::string_view message)
{
  if(m_config.debugLogRXTX)
  {
    std::string msg{message};
    std::replace(msg.begin(), msg.end(), '\n', ';');
    EventLoop::call([this, msg](){ Log::log(m_logId, LogMessage::D2002_RX_X, msg); });
  }

  if(startsWith(message, "<REPLY "))
  {

  }
  else if(startsWith(message, "<EVENT "))
  {

  }
}

void Kernel::emergencyStop()
{
  m_ioContext.post(
    [this]()
    {
      if(m_emergencyStop != TriState::True)
        send(set(ObjectId::ecos, {Option::stop}));
    });
}

void Kernel::go()
{
  m_ioContext.post(
    [this]()
    {
      if(m_emergencyStop != TriState::False)
        send(set(ObjectId::ecos, {Option::go}));
    });
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
