/**
 * server/src/network/websocketconnection.cpp
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

#include "websocketconnection.hpp"
#include <cassert>
#include <thread>
#include "server.hpp"
#include "../core/eventloop.hpp"
#include "../log/log.hpp"

namespace {

std::string createId(const boost::beast::websocket::stream<boost::beast::tcp_stream>& ws, std::string_view idPrefix)
{
  auto& socket = boost::beast::get_lowest_layer(ws).socket();
  return std::string(idPrefix)
    .append("[")
    .append(socket.remote_endpoint().address().to_string())
    .append(":")
    .append(std::to_string(socket.remote_endpoint().port()))
    .append("]");
}

}

WebSocketConnection::WebSocketConnection(Server& server, std::shared_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> ws, std::string_view idPrefix)
  : m_server{server}
  , m_ws(std::move(ws))
  , id{createId(*m_ws, idPrefix)}
{
  assert(isServerThread());
  assert(m_ws);
}

WebSocketConnection::~WebSocketConnection()
{
  assert(isEventLoopThread());
  assert(!m_ws->is_open());
}

void WebSocketConnection::start()
{
  assert(isServerThread());
  doRead();
}

void WebSocketConnection::disconnect()
{
  assert(isEventLoopThread());

  m_server.m_ioContext.post(
    [this]()
    {
      if(m_ws->is_open())
      {
        boost::system::error_code ec;
        m_ws->close(boost::beast::websocket::close_code::normal, ec);
      }

      EventLoop::call(
        [this]()
        {
          m_server.connectionGone(shared_from_this());
        });
    });
}

#ifndef NDEBUG
bool WebSocketConnection::isServerThread() const
{
  return std::this_thread::get_id() == m_server.threadId();
}
#endif

boost::asio::io_context& WebSocketConnection::ioContext()
{
  return m_server.m_ioContext;
}

void WebSocketConnection::connectionLost()
{
  assert(isEventLoopThread());
  Log::log(id, LogMessage::I1004_CONNECTION_LOST);
  disconnect();
}
