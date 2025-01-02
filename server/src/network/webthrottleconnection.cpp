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
#include "../core/eventloop.hpp"
#include "../log/log.hpp"
WebThrottleConnection::WebThrottleConnection(Server& server, std::shared_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> ws)
  : WebSocketConnection(server, std::move(ws), "webthrottle")
{
  assert(isServerThread());

  m_ws->binary(false);
}

WebThrottleConnection::~WebThrottleConnection()
{
  assert(isEventLoopThread());
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
  (void)message;
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
