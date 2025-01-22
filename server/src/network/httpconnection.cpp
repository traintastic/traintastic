/**
 * server/src/network/httpconnection.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "httpconnection.hpp"
#include "server.hpp"
#include <boost/beast/http/read.hpp>
#include <boost/beast/websocket/rfc6455.hpp>

HTTPConnection::HTTPConnection(std::shared_ptr<Server> server, boost::asio::ip::tcp::socket&& socket)
  : m_server{std::move(server)}
  , m_stream(std::move(socket))
{
}

void HTTPConnection::start()
{
  doRead();
}

void HTTPConnection::doRead()
{
  m_request = {}; // clear request, otherwise the operation behavior is undefined

  m_stream.expires_after(std::chrono::seconds(30));

  boost::beast::http::async_read(m_stream, m_buffer, m_request,
    [this, self = shared_from_this()](boost::beast::error_code readError, size_t /*bytesTransferred*/)
    {
      if(readError)
      {
        if(readError == boost::beast::http::error::end_of_stream)
        {
          return doClose();
        }
        return;
      }

      const bool keepAlive = m_request.keep_alive();

      if(boost::beast::websocket::is_upgrade(m_request))
      {
        if(!m_server->handleWebSocketUpgradeRequest(std::move(m_request), m_stream))
        {
          self->doRead(); // no upgrade, handle next request
        }
        return;
      }

      auto response = m_server->handleHTTPRequest(std::move(m_request));

      boost::beast::async_write(m_stream, std::move(response),
        [self, keepAlive](boost::beast::error_code writeError, size_t /*bytesTransferred*/)
        {
          if(writeError)
          {
            return;
          }

          if(!keepAlive)
          {
            return self->doClose();
          }

          self->doRead(); // handle next request
        });
  });
}

void HTTPConnection::doClose()
{
  boost::beast::error_code ec;
  m_stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
}
