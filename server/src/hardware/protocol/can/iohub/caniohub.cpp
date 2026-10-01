/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "caniohub.hpp"
#include "caniohubconnection.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../log/logmessageexception.hpp"

namespace CAN {

IOHub::IOHub(boost::asio::io_context& ioContext, std::string logId, bool localhostOnly, uint16_t port)
  : m_acceptor{ioContext}
  , m_logId{logId}
  , m_localhostOnly{localhostOnly}
  , m_port{port}
{
}

IOHub::~IOHub()
{
  LOG_DEBUG("IOHub::~IOHub");
}

void IOHub::start(OnReceive onReceive)
{
  assert(onReceive);
  m_onReceive = std::move(onReceive);

  boost::asio::ip::tcp::endpoint endpoint(m_localhostOnly ? boost::asio::ip::address_v4::loopback() : boost::asio::ip::address_v4::any(), m_port);

  boost::system::error_code ec;
  m_acceptor.open(endpoint.protocol(), ec);
  if(ec)
  {
    throw LogMessageException(LogMessage::E2004_SOCKET_OPEN_FAILED_X, ec);
  }

  m_acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
  if(ec)
  {
    throw LogMessageException(LogMessage::E2028_SOCKET_SETSOCKOPT_FAILED_X, ec);
  }

  m_acceptor.bind(endpoint, ec);
  if(ec)
  {
    throw LogMessageException(LogMessage::E2006_SOCKET_BIND_FAILED_X, ec);
  }

  m_acceptor.listen(5, ec);
  if(ec)
  {
    throw LogMessageException(LogMessage::E2030_TCP_SOCKET_LISTEN_FAILED_X, ec);
  }

  EventLoop::call(
    [id=m_logId, ep=m_acceptor.local_endpoint()]()
    {
      Log::log(id, LogMessage::N2008_HUB_LISTENING_AT_X_X, ep.address().to_string(), ep.port());
    });

  accept();
}

void IOHub::stop()
{
  boost::system::error_code ec;
  m_acceptor.cancel(ec);
  m_acceptor.close(ec);

  while(!m_connections.empty())
  {
    m_connections.front()->stop();
    m_connections.pop_front();
  }
}

void IOHub::send(const Message& message)
{
  for(auto& connection : m_connections)
  {
    connection->send(message);
  }
}

void IOHub::accept()
{
  m_acceptor.async_accept(
    [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
    {
      if(!ec)
      {
        auto& connection = m_connections.emplace_back(newConnection(std::move(socket)));
        connection->start();

        accept();
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [id=m_logId, ec]()
          {
            Log::log(id, LogMessage::E2029_TCP_ACCEPT_ERROR_X, ec);
          });
      }
    });
}

void IOHub::receive(const Message& message, IOHubConnection& source)
{
  m_onReceive(message);

  for(auto& connection : m_connections)
  {
    if(connection.get() != &source)
    {
      connection->send(message);
    }
  }
}

}
