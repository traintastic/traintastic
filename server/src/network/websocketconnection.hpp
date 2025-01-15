/**
 * server/src/network/websocketconnection.hpp
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

#ifndef TRAINTASTIC_SERVER_NETWORK_WEBSOCKETCONNECTION_HPP
#define TRAINTASTIC_SERVER_NETWORK_WEBSOCKETCONNECTION_HPP

#include <memory>
#include <boost/beast/core/tcp_stream.hpp>
#pragma GCC diagnostic push
#ifdef __linux__
  #pragma GCC diagnostic ignored "-Wstringop-overflow" // FIXME: for boost 1.81 using GCC 12 on rasbian armhf
#endif
#include <boost/beast/websocket/stream.hpp>
#pragma GCC diagnostic pop

class Server;

class WebSocketConnection : public std::enable_shared_from_this<WebSocketConnection>
{
protected:
  Server& m_server;
  std::shared_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> m_ws;

#ifndef NDEBUG
  bool isServerThread() const;
#endif

  boost::asio::io_context& ioContext();

  virtual void doRead() = 0;
  virtual void doWrite() = 0;

  void connectionLost();

public:
  const std::string id;

  WebSocketConnection(Server& server, std::shared_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> ws, std::string_view idPrefix);
  virtual ~WebSocketConnection();

  virtual void start();

  virtual void disconnect();
};

#endif
