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
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../hardware/decoder/decoder.hpp"
#include "../throttle/webthrottle.hpp"
#include "../log/log.hpp"
#include "../train/train.hpp"
#include "../train/trainerror.hpp"
#include "../train/trainlist.hpp"
#include "../train/trainvehiclelist.hpp"

WebThrottleConnection::WebThrottleConnection(Server& server, std::shared_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> ws)
  : WebSocketConnection(server, std::move(ws), "webthrottle")
{
  assert(isServerThread());

  m_ws->binary(false);
}

WebThrottleConnection::~WebThrottleConnection()
{
  assert(isEventLoopThread());

  // disconnect all signals:
  m_traintasticPropertyChanged.disconnect();
  m_trainConnections.clear();
  m_throttleConnections.clear();

  // destroy all throttles:
  for(auto& it : m_throttles)
  {
    it.second->destroy();
    it.second.reset();
  }
}

void WebThrottleConnection::start()
{
  WebSocketConnection::start();

  EventLoop::call(
    [this]()
    {
      m_traintasticPropertyChanged = Traintastic::instance->propertyChanged.connect(
        [this](BaseProperty& property)
        {
          if(property.name() == "world")
          {
            assert(m_throttles.empty());
            sendWorld(static_cast<ObjectProperty<World>&>(property).value());
          }
        });

      sendWorld(Traintastic::instance->world.value());
    });
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
    else if(action == "estop_all")
    {
      for(const auto& it : m_throttles)
      {
        it.second->emergencyStop();
      }
    }
  }
  else
  {
    const auto& throttle = getThrottle(throttleId);

    if(!throttle)
    {
      if(Traintastic::instance->world.value())
      {
        sendError(throttleId, "No world loaded", "no_world_loaded");
      }
      else
      {
        sendError(throttleId, "Failed to create throttle");
      }
      return;
    }

    if(action == "acquire")
    {
      auto train = std::dynamic_pointer_cast<Train>(world->getObjectById(message.value("train_id", "")));
      if(train)
      {
        nlohmann::json object;

        const auto ec = throttle->acquire(train, message.value("steal", false));
        if(!ec)
        {
          m_trainConnections.erase(throttleId);

          m_trainConnections.emplace(throttleId, train->propertyChanged.connect(
            [this, throttleId](BaseProperty& property)
            {
              const auto name = property.name();
              if(name == "direction" || name == "speed" || name == "throttle_speed" || name == "is_stopped")
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
          object.emplace("is_stopped", train->isStopped.toJSON());
          object.emplace("speed", train->speed.toJSON());
          object.emplace("throttle_speed", train->throttleSpeed.toJSON());

          auto functions = nlohmann::json::array();
          for(const auto& vehicle : *train->vehicles)
          {
            if(const auto& decoder = vehicle->decoder.value(); decoder && !decoder->functions->empty())
            {
              auto group = nlohmann::json::object();
              group.emplace("id", vehicle->id.toJSON());
              group.emplace("name", vehicle->name.toJSON());
              auto items = nlohmann::json::array();
              for(const auto& function : *decoder->functions)
              {
                m_trainConnections.emplace(throttleId, function->propertyChanged.connect(
                  [this, throttleId, vehicleId=vehicle->id.value()](BaseProperty& property)
                  {
                    if(property.name() == "value")
                    {
                      const auto& decoderFunction = static_cast<const DecoderFunction&>(property.object());
                      auto event = nlohmann::json::object();
                      event.emplace("event", "function_value");
                      event.emplace("throttle_id", throttleId);
                      event.emplace("vehicle_id", vehicleId);
                      event.emplace("number", decoderFunction.number.toJSON());
                      event.emplace("value", property.toJSON());
                      sendMessage(event);
                    }
                  }));

                auto item = nlohmann::json::object();
                item.emplace("number", function->number.toJSON());
                item.emplace("name", function->name.toJSON());
                item.emplace("type", function->type.toJSON());
                item.emplace("function", function->function.toJSON());
                item.emplace("value", function->value.toJSON());
                items.emplace_back(item);
              }
              group.emplace("items", items);
              functions.emplace_back(group);
            }
          }
          object.emplace("functions", functions);

          auto response = nlohmann::json::object();
          response.emplace("event", "train");
          response.emplace("throttle_id", throttleId);
          response.emplace("train", object);
          sendMessage(response);
        }
        else // error
        {
          sendError(throttleId, ec);
        }
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
        throttle->stop(message.value("immediate", false));
      }
      else if(action == "faster")
      {
        throttle->faster(message.value("immediate", false));
      }
      else if(action == "slower")
      {
        throttle->slower(message.value("immediate", false));
      }
      else if(action == "reverse" || action == "forward")
      {
        const auto direction = (action == "forward") ? Direction::Forward : Direction::Reverse;
        if(const auto ec = throttle->train->setDirection(*throttle, direction); ec)
        {
          sendError(throttleId, ec);
        }
      }
      else if(action == "release")
      {
        throttle->release(message.value("stop", true));
        released(throttleId);
      }
      else if(action == "toggle_function")
      {
        const auto vehicleId = message.value<std::string_view>("vehicle_id", {});
        const auto functionNumber = message.value<uint32_t>("function_number", 0);

        for(const auto& vehicle : *throttle->train->vehicles)
        {
          if(vehicle->id.value() == vehicleId && vehicle->decoder)
          {
            if(const auto& function = vehicle->decoder->getFunction(functionNumber))
            {
              function->value = !function->value;
            }
          }
        }
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

void WebThrottleConnection::sendError(uint32_t throttleId, std::string_view text, std::string_view tag)
{
  assert(isEventLoopThread());

  auto error = nlohmann::json::object();
  error.emplace("event", "message");
  error.emplace("throttle_id", throttleId);
  error.emplace("type", "error");
  if(!tag.empty())
  {
    error.emplace("tag", tag);
  }
  error.emplace("text", text);
  sendMessage(error);
}

void WebThrottleConnection::sendError(uint32_t throttleId, std::error_code ec)
{
  assert(isEventLoopThread());

  if(ec == TrainError::AlreadyAcquired)
  {
    sendError(throttleId, ec.message(), "already_acquired");
  }
  else if(ec == TrainError::CanNotActivateTrain)
  {
    sendError(throttleId, ec.message(), "can_not_activate_train");
  }
  else if(ec == TrainError::TrainMustBeStoppedToChangeDirection)
  {
    sendError(throttleId, ec.message(), "train_must_be_stopped_to_change_direction");
  }
  else
  {
    sendError(throttleId, ec.message());
  }
}

void WebThrottleConnection::sendWorld(const std::shared_ptr<World>& world)
{
  assert(isEventLoopThread());

  auto event = nlohmann::json::object();
  event.emplace("event", "world");
  if(world)
  {
    event.emplace("name", world->name.toJSON());
  }
  else
  {
    event.emplace("name", nullptr);
  }
  sendMessage(event);
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
      m_throttleConnections.emplace(throttleId, it->second->onDestroying.connect(
        [this, throttleId](Object& /*object*/)
        {
          released(throttleId);
          m_throttleConnections.erase(throttleId);
          m_throttles.erase(throttleId);
        }));
      m_throttleConnections.emplace(throttleId, it->second->released.connect(
        [this, throttleId]()
        {
          released(throttleId);
        }));
      return it->second;
    }
  }

  return noThrottle;
}

void WebThrottleConnection::released(uint32_t throttleId)
{
  assert(isEventLoopThread());

  auto response = nlohmann::json::object();
  response.emplace("event", "train");
  response.emplace("throttle_id", throttleId);
  response.emplace("train", nullptr);
  sendMessage(response);
}
