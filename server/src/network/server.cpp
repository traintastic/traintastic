/**
 * server/src/network/server.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#include "server.hpp"
#include <traintastic/network/message.hpp>
#include <version.hpp>
#include "connection.hpp"
#include "../core/eventloop.hpp"
#include "../log/log.hpp"
#include "../log/logmessageexception.hpp"
#include "../utils/setthreadname.hpp"

#define IS_SERVER_THREAD (std::this_thread::get_id() == m_thread.get_id())

Server::Server(bool localhostOnly, uint16_t port, bool discoverable)
  : m_ioContext{1}
  , m_acceptor{m_ioContext}
  , m_socketUDP{m_ioContext}
  , m_localhostOnly{localhostOnly}
{
  assert(isEventLoopThread());

  boost::system::error_code ec;
  boost::asio::ip::tcp::endpoint endpoint(localhostOnly ? boost::asio::ip::address_v4::loopback() : boost::asio::ip::address_v4::any(), port);

  m_acceptor.open(endpoint.protocol(), ec);
  if(ec)
    throw LogMessageException(LogMessage::F1001_OPENING_TCP_SOCKET_FAILED_X, ec);

  m_acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
  if(ec)
    throw LogMessageException(LogMessage::F1002_TCP_SOCKET_ADDRESS_REUSE_FAILED_X, ec);

  m_acceptor.bind(endpoint, ec);
  if(ec)
    throw LogMessageException(LogMessage::F1003_BINDING_TCP_SOCKET_FAILED_X, ec);

  m_acceptor.listen(5, ec);
  if(ec)
    throw LogMessageException(LogMessage::F1004_TCP_SOCKET_LISTEN_FAILED_X, ec);

  if(discoverable)
  {
    if(port == defaultPort)
    {
      m_socketUDP.open(boost::asio::ip::udp::v4(), ec);
      if(ec)
        throw LogMessageException(LogMessage::F1005_OPENING_UDP_SOCKET_FAILED_X, ec);

      m_socketUDP.set_option(boost::asio::socket_base::reuse_address(true), ec);
      if(ec)
        throw LogMessageException(LogMessage::F1006_UDP_SOCKET_ADDRESS_REUSE_FAILED_X, ec);

      m_socketUDP.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), defaultPort), ec);
      if(ec)
        throw LogMessageException(LogMessage::F1007_BINDING_UDP_SOCKET_FAILED_X, ec);

      Log::log(id, LogMessage::N1005_DISCOVERY_ENABLED);
    }
    else
    {
      Log::log(id, LogMessage::W1001_DISCOVERY_DISABLED_ONLY_ALLOWED_ON_PORT_X, defaultPort);
      discoverable = false;
    }
  }
  else
    Log::log(id, LogMessage::N1006_DISCOVERY_DISABLED);

  Log::log(id, LogMessage::N1007_LISTENING_AT_X_X, m_acceptor.local_endpoint().address().to_string(), m_acceptor.local_endpoint().port());

  m_thread = std::thread(
    [this]()
    {
      setThreadName("server");
      auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
      m_ioContext.run();
    });

  m_ioContext.post(
    [this, discoverable]()
    {
      if(discoverable)
        doReceive();

      doAccept();
    });
}

Server::~Server()
{
  assert(isEventLoopThread());

  if(!m_ioContext.stopped())
  {
    m_ioContext.post(
      [this]()
      {
        boost::system::error_code ec;
        if(m_acceptor.cancel(ec))
          Log::log(id, LogMessage::E1008_SOCKET_ACCEPTOR_CANCEL_FAILED_X, ec);

        m_acceptor.close();

        m_socketUDP.close();
      });

    m_ioContext.stop();
  }

  if(m_thread.joinable())
    m_thread.join();

  while(!m_connections.empty())
    m_connections.front()->disconnect();
}

void Server::connectionGone(const std::shared_ptr<Connection>& connection)
{
  assert(isEventLoopThread());

  m_connections.erase(std::find(m_connections.begin(), m_connections.end(), connection));
}

void Server::doReceive()
{
  assert(IS_SERVER_THREAD);

  m_socketUDP.async_receive_from(boost::asio::buffer(m_udpBuffer), m_remoteEndpoint,
    [this](const boost::system::error_code& ec, std::size_t bytesReceived)
    {
      if(!ec)
      {
        if(bytesReceived == sizeof(Message::Header))
        {
          Message message(*reinterpret_cast<Message::Header*>(m_udpBuffer.data()));

          if(!m_localhostOnly || m_remoteEndpoint.address().is_loopback())
          {
            if(message.dataSize() == 0)
            {
              std::unique_ptr<Message> response = processMessage(message);
              if(response)
              {
                m_socketUDP.async_send_to(boost::asio::buffer(**response, response->size()), m_remoteEndpoint,
                  [this](const boost::system::error_code& /*ec*/, std::size_t /*bytesTransferred*/)
                  {
                    doReceive();
                  });
                return;
              }
            }
          }
        }
        doReceive();
      }
      else
        Log::log(id, LogMessage::E1003_UDP_RECEIVE_ERROR_X, ec.message());
    });
}

std::unique_ptr<Message> Server::processMessage(const Message& message)
{
  assert(IS_SERVER_THREAD);

  if(message.command() == Message::Command::Discover && message.isRequest())
  {
    std::unique_ptr<Message> response = Message::newResponse(message.command(), message.requestId());
    response->write(boost::asio::ip::host_name());
    response->write<uint16_t>(TRAINTASTIC_VERSION_MAJOR);
    response->write<uint16_t>(TRAINTASTIC_VERSION_MINOR);
    response->write<uint16_t>(TRAINTASTIC_VERSION_PATCH);
    assert(response->size() <= 1500); // must fit in a UDP packet
    return response;
  }

  return {};
}

void Server::doAccept()
{
  assert(IS_SERVER_THREAD);

  assert(!m_socketTCP);
  m_socketTCP = std::make_shared<boost::asio::ip::tcp::socket>(m_ioContext);

  m_acceptor.async_accept(*m_socketTCP,
    [this](boost::system::error_code ec)
    {
      if(!ec)
      {
        EventLoop::call(
          [this, socket=std::move(m_socketTCP)]()
          {
            try
            {
              m_connections.emplace_back(std::make_shared<Connection>(*this, std::move(*socket)));
            }
            catch(const std::exception& e)
            {
              Log::log(id, LogMessage::C1002_CREATING_CONNECTION_FAILED_X, e.what());
            }
          });

        doAccept();
      }
      else
      {
        Log::log(id, LogMessage::E1004_TCP_ACCEPT_ERROR_X, ec.message());
        m_socketTCP.reset();
      }
    });
}
