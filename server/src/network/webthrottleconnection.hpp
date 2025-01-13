/**
 * server/src/network/webthrottleconnection.hpp
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

#ifndef TRAINTASTIC_SERVER_NETWORK_WEBTHROTTLECONNECTION_HPP
#define TRAINTASTIC_SERVER_NETWORK_WEBTHROTTLECONNECTION_HPP

#include <queue>
#include <map>
#include <boost/asio.hpp>
#include <boost/signals2/connection.hpp>
#include <nlohmann/json.hpp>
#include "websocketconnection.hpp"

class WebThrottle;

class WebThrottleConnection : public WebSocketConnection
{
protected:
  boost::beast::flat_buffer m_readBuffer;
  std::queue<std::string> m_writeQueue;
  std::map<uint32_t, std::shared_ptr<WebThrottle>> m_throttles;
  std::map<uint32_t, boost::signals2::scoped_connection> m_throttleReleased;
  std::map<uint32_t, boost::signals2::scoped_connection> m_trainPropertyChanged;

  void doRead() final;
  void doWrite() final;

  void processMessage(const nlohmann::json& message);
  void sendMessage(const nlohmann::json& message);

  const std::shared_ptr<WebThrottle>& getThrottle(uint32_t throttleId);

  void released(uint32_t throttleId);

public:
  const std::string id;

  WebThrottleConnection(Server& server, std::shared_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> ws);
  virtual ~WebThrottleConnection();
};

#endif
