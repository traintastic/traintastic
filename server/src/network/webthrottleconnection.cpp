/**
 * server/src/network/webthrottleconnection.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#include "webthrottleconnection.hpp"
#include "server.hpp"
#include "../traintastic/traintastic.hpp"
#include "../core/eventloop.hpp"
#include "../core/objectproperty.tpp"
#include "../hardware/throttle/webthrottle.hpp"
#include "../log/log.hpp"
#include "../train/train.hpp"
#include "../train/trainerror.hpp"
#include "../train/trainlist.hpp"

WebThrottleConnection::WebThrottleConnection(Server& server, std::shared_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> ws)
  : WebSocketConnection(server, std::move(ws), "webthrottle")
{
  assert(isServerThread());

  m_ws->binary(false);
}

WebThrottleConnection::~WebThrottleConnection()
{
  assert(isEventLoopThread());

  for(auto& it : m_throttles)
  {
    it.second->destroy();
    it.second.reset();
  }
}

void WebThrottleConnection::doRead()
{
  assert(isServerThread());

  m_ws->async_read(m_readBuffer,
    [this, weak=weak_from_this()](const boost::system::error_code& ec, std::size_t /*bytesReceived*/)
    {
      if(weak.expired())
        return;

      if(!ec)
      {
        std::string_view sv(static_cast<const char*>(m_readBuffer.cdata().data()), m_readBuffer.size());

        EventLoop::call(
          [this, message=nlohmann::json::parse(sv)]()
          {
            processMessage(message);
          });
        m_readBuffer.consume(m_readBuffer.size());
        doRead();
      }
      else if(
          ec == boost::asio::error::eof ||
          ec == boost::asio::error::connection_aborted ||
          ec == boost::asio::error::connection_reset)
      {
        // Socket read failed (The WebSocket stream was gracefully closed at both endpoints)
        EventLoop::call(std::bind(&WebThrottleConnection::connectionLost, this));
      }
      else
      {
        Log::log(id, LogMessage::E1007_SOCKET_READ_FAILED_X, ec);
        EventLoop::call(std::bind(&WebThrottleConnection::disconnect, this));
      }
    });
}

void WebThrottleConnection::doWrite()
{
  assert(isServerThread());

  m_ws->async_write(boost::asio::buffer(m_writeQueue.front().data(), m_writeQueue.front().size()),
    [this, weak=weak_from_this()](const boost::system::error_code& ec, std::size_t /*bytesTransferred*/)
    {
      if(weak.expired())
        return;

      if(!ec)
      {
        m_writeQueue.pop();
        if(!m_writeQueue.empty())
          doWrite();
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        Log::log(id, LogMessage::E1006_SOCKET_WRITE_FAILED_X, ec);
        EventLoop::call(std::bind(&WebThrottleConnection::disconnect, this));
      }
    });
}

void WebThrottleConnection::processMessage(const nlohmann::json& message)
{
  assert(isEventLoopThread());

  const auto& world = Traintastic::instance->world.value();
  const auto action = message.value("action", "");
  const auto throttleId = message.value<uint32_t>("throttle_id", 0);

  if(throttleId == 0)
  {
    if(action == "get_train_list")
    {
      auto response = nlohmann::json::object();
      response.emplace("event", "train_list");
      auto list = nlohmann::json::array();
      if(world)
      {
        for(const auto& train : *world->trains)
        {
          auto item = nlohmann::json::object();
          item.emplace("id", train->id.value());
          item.emplace("name", train->name.value());
          list.emplace_back(item);
        }
      }
      response.emplace("list", list);
      sendMessage(response);
    }
  }
  else
  {
    const auto& throttle = getThrottle(throttleId);

    if(action == "acquire")
    {
      auto train = std::dynamic_pointer_cast<Train>(world->getObjectById(message.value("train_id", "")));
      if(train)
      {
        nlohmann::json object;

        const auto ec = throttle->acquire(train, message.value("steal", false));
        if(!ec)
        {
          m_trainPropertyChanged.emplace(throttleId, train->propertyChanged.connect(
            [this, throttleId](BaseProperty& property)
            {
              const auto name = property.name();
              if(name == "direction" || name == "speed" || name == "throttle_speed")
              {
                auto event = nlohmann::json::object();
                event.emplace("event", name);
                event.emplace("throttle_id", throttleId);
                if(dynamic_cast<AbstractUnitProperty*>(&property))
                {
                  event.update(property.toJSON());
                }
                else
                {
                  event.emplace("value", property.toJSON());
                }
                sendMessage(event);
              }
            }));

          object = nlohmann::json::object();
          object.emplace("id", train->id.toJSON());
          object.emplace("name", train->name.toJSON());
          object.emplace("direction", train->direction.toJSON());
          object.emplace("speed", train->speed.toJSON());
          object.emplace("throttle_speed", train->throttleSpeed.toJSON());
        }
        else // error
        {
          auto error = nlohmann::json::object();
          error.emplace("event", "message");
          error.emplace("throttle_id", throttleId);
          error.emplace("type", "error");
          if(ec == TrainError::AlreadyAcquired)
          {
            error.emplace("tag", "already_acquired");
          }
          error.emplace("text", ec.message());
          sendMessage(error);
        }
        auto response = nlohmann::json::object();
        response.emplace("event", "train");
        response.emplace("throttle_id", throttleId);
        response.emplace("train", object);
        sendMessage(response);
      }
    }
    else if(action == "set_name")
    {
      throttle->name = message.value("value", "");
    }
    else if(throttle->acquired())
    {
      if(action == "estop")
      {
        throttle->emergencyStop();
      }
      else if(action == "stop")
      {
        throttle->stop();
      }
      else if(action == "faster")
      {
        throttle->faster();
      }
      else if(action == "slower")
      {
        throttle->slower();
      }
      else if(action == "reverse")
      {
        throttle->setDirection(Direction::Reverse);
      }
      else if(action == "forward")
      {
        throttle->setDirection(Direction::Forward);
      }
      else if(action == "release")
      {
        throttle->release(message.value("stop", true));

        m_trainPropertyChanged.erase(throttleId);

        auto response = nlohmann::json::object();
        response.emplace("event", "train");
        response.emplace("throttle_id", throttleId);
        response.emplace("train", nullptr);
        sendMessage(response);
      }
    }
  }
}

void WebThrottleConnection::sendMessage(const nlohmann::json& message)
{
  assert(isEventLoopThread());

  ioContext().post(
    [this, msg=message.dump()]()
    {
      const bool wasEmpty = m_writeQueue.empty();
      m_writeQueue.push(msg);
      if(wasEmpty)
      {
        doWrite();
      }
    });
}

const std::shared_ptr<WebThrottle>& WebThrottleConnection::getThrottle(uint32_t throttleId)
{
  assert(isEventLoopThread());

  static const std::shared_ptr<WebThrottle> noThrottle;

  if(auto it = m_throttles.find(throttleId); it != m_throttles.end())
  {
    return it->second;
  }

  if(const auto& world = Traintastic::instance->world.value())
  {
    auto [it, inserted] = m_throttles.emplace(throttleId, WebThrottle::create(*world));
    if(inserted) /*[[likely]]*/
    {
      m_throttleReleased.emplace(throttleId, it->second->released.connect(
        [this, throttleId]()
        {
          auto response = nlohmann::json::object();
          response.emplace("event", "train");
          response.emplace("throttle_id", throttleId);
          response.emplace("train", nullptr);
          sendMessage(response);
        }));
      return it->second;
    }
  }

  return noThrottle;
}
