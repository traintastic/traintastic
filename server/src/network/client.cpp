/**
 * server/src/network/client.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#include "client.hpp"
#include "server.hpp"
#include "../traintastic/traintastic.hpp"
#include "../core/eventloop.hpp"
#include "session.hpp"
#include "../log/log.hpp"

#ifndef NDEBUG
  #define IS_SERVER_THREAD (std::this_thread::get_id() == m_server.threadId())
#endif

std::string clientId(const boost::asio::ip::tcp::socket& socket)
{
  return
    std::string("client[")
      .append(socket.remote_endpoint().address().to_string())
      .append(":")
      .append(std::to_string(socket.remote_endpoint().port()))
      .append("]");
}

Client::Client(Server& server, boost::asio::ip::tcp::socket socket)
  : m_server{server}
  , m_socket(std::move(socket))
  , m_id{clientId(m_socket)}
  , m_authenticated{false}
{
  assert(isEventLoopThread());

  Log::log(m_id, LogMessage::I1003_CLIENT_CONNECTED);

  m_server.m_ioContext.post(
    [this]()
    {
      m_socket.set_option(boost::asio::socket_base::linger(true, 0));
      m_socket.set_option(boost::asio::ip::tcp::no_delay(true));

      doReadHeader();
    });
}

Client::~Client()
{
  assert(isEventLoopThread());
  assert(!m_session);
  assert(!m_socket.is_open());
}

void Client::doReadHeader()
{
  assert(IS_SERVER_THREAD);

  boost::asio::async_read(m_socket,
    boost::asio::buffer(&m_readBuffer.header, sizeof(m_readBuffer.header)),
      [this, weak=weak_from_this()](const boost::system::error_code& ec, std::size_t /*bytesReceived*/)
      {
        if(weak.expired())
          return;

        if(!ec)
        {
          m_readBuffer.message.reset(new Message(m_readBuffer.header));
          if(m_readBuffer.message->dataSize() == 0)
          {
            if(m_readBuffer.message->command() != Message::Command::Ping)
              EventLoop::call(&Client::processMessage, this, m_readBuffer.message);
            else
              {} // TODO: ping hier replyen
            m_readBuffer.message.reset();
            doReadHeader();
          }
          else
            doReadData();
        }
        else if(ec == boost::asio::error::eof || ec == boost::asio::error::connection_aborted || ec == boost::asio::error::connection_reset)
        {
          EventLoop::call(std::bind(&Client::connectionLost, this));
        }
        else if(ec != boost::asio::error::operation_aborted)
        {
          Log::log(m_id, LogMessage::E1007_SOCKET_READ_FAILED_X, ec);
          EventLoop::call(std::bind(&Client::disconnect, this));
        }
      });
}

void Client::doReadData()
{
  assert(IS_SERVER_THREAD);

  boost::asio::async_read(m_socket,
    boost::asio::buffer(m_readBuffer.message->data(), m_readBuffer.message->dataSize()),
      //m_strand.wrap(
        [this, weak=weak_from_this()](const boost::system::error_code& ec, std::size_t /*bytesReceived*/)
        {
          if(weak.expired())
            return;

          if(!ec)
          {
            if(m_readBuffer.message->command() != Message::Command::Ping)
              EventLoop::call(&Client::processMessage, this, m_readBuffer.message);
            else
            {} // TODO: ping hier replyen
            m_readBuffer.message.reset();
            doReadHeader();
          }
          else if(ec == boost::asio::error::eof || ec == boost::asio::error::connection_aborted || ec == boost::asio::error::connection_reset)
          {
            EventLoop::call(std::bind(&Client::connectionLost, this));
          }
          else if(ec != boost::asio::error::operation_aborted)
          {
            Log::log(m_id, LogMessage::E1007_SOCKET_READ_FAILED_X, ec);
            EventLoop::call(std::bind(&Client::disconnect, this));
          }
        });
}

void Client::doWrite()
{
  assert(IS_SERVER_THREAD);

  boost::asio::async_write(m_socket, boost::asio::buffer(**m_writeQueue.front(), m_writeQueue.front()->size()),
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
        Log::log(m_id, LogMessage::E1006_SOCKET_WRITE_FAILED_X, ec);
        EventLoop::call(std::bind(&Client::disconnect, this));
      }
    });
}

void Client::processMessage(const std::shared_ptr<Message> message)
{
  assert(isEventLoopThread());

  if(m_authenticated && m_session)
  {
    if(m_session->processMessage(*message))
      return;
  }
  else if(m_authenticated && !m_session)
  {
    if(message->command() == Message::Command::NewSession && message->type() == Message::Type::Request)
    {
      m_session = std::make_shared<Session>(shared_from_this());
      auto response = Message::newResponse(message->command(), message->requestId());
      response->write(m_session->uuid());
      m_session->writeObject(*response, Traintastic::instance);
      sendMessage(std::move(response));
      return;
    }
  }
  else
  {
    if(message->command() == Message::Command::Login && message->type() == Message::Type::Request)
    {
      m_authenticated = true; // oke for now, login can be added later :)
      sendMessage(Message::newResponse(message->command(), message->requestId()));
      return;
    }
  }

  if(message->type() == Message::Type::Request)
  {
    //assert(false);
    sendMessage(Message::newErrorResponse(message->command(), message->requestId(), Message::ErrorCode::InvalidCommand));
  }
}

void Client::sendMessage(std::unique_ptr<Message> message)
{
  assert(isEventLoopThread());

  m_server.m_ioContext.post(
    [this, msg=std::make_shared<std::unique_ptr<Message>>(std::move(message))]()
    {
      const bool wasEmpty = m_writeQueue.empty();
      m_writeQueue.emplace(std::move(*msg));
      if(wasEmpty)
        doWrite();
    });
}

void Client::connectionLost()
{
  assert(isEventLoopThread());

  Log::log(m_id, LogMessage::I1004_CONNECTION_LOST);
  disconnect();
}

void Client::disconnect()
{
  assert(isEventLoopThread());

  m_session.reset();

  m_server.m_ioContext.post(
    [this]()
    {
      if(m_socket.is_open())
      {
        boost::system::error_code ec;
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        if(ec && ec != boost::asio::error::not_connected)
          Log::log(m_id, LogMessage::E1005_SOCKET_SHUTDOWN_FAILED_X, ec);
        m_socket.close();
      }

      EventLoop::call(
        [this]()
        {
          m_server.clientGone(shared_from_this());
        });
    });
}
