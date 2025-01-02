/**
 * server/src/network/clientclientconnection.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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

#include "clientconnection.hpp"
#include "server.hpp"
#include "../traintastic/traintastic.hpp"
#include "../core/eventloop.hpp"
#include "session.hpp"
#include "../log/log.hpp"

ClientConnection::ClientConnection(Server& server, std::shared_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> ws)
  : WebSocketConnection(server, std::move(ws), "client")
  , m_authenticated{false}
{
  assert(isServerThread());
  m_ws->binary(true);
}

ClientConnection::~ClientConnection()
{
  assert(isEventLoopThread());
  assert(!m_session);
}

void ClientConnection::doRead()
{
  assert(isServerThread());

  m_ws->async_read(m_readBuffer,
    [this, weak=weak_from_this()](const boost::system::error_code& ec, std::size_t /*bytesReceived*/)
    {
      if(weak.expired())
        return;

      if(!ec)
      {
        while(m_readBuffer.size() >= sizeof(Message::Header))
        {
          const Message::Header& header = *reinterpret_cast<const Message::Header*>(m_readBuffer.cdata().data());
          if(m_readBuffer.size() >= sizeof(Message::Header) + header.dataSize)
          {
            auto message = std::make_shared<Message>(header);
            if(header.dataSize != 0)
            {
              std::memcpy(message->data(), static_cast<const std::byte*>(m_readBuffer.cdata().data()) + sizeof(header), message->dataSize());
            }
            EventLoop::call(&ClientConnection::processMessage, this, message);
            m_readBuffer.consume(sizeof(header) + header.dataSize);
          }
          else
          {
            break;
          }
        }
        doRead();
      }
      else if(ec == boost::asio::error::eof || ec == boost::asio::error::connection_aborted || ec == boost::asio::error::connection_reset)
      {
        EventLoop::call(std::bind(&ClientConnection::connectionLost, this));
      }
      else
      {
        Log::log(id, LogMessage::E1007_SOCKET_READ_FAILED_X, ec);
        EventLoop::call(std::bind(&ClientConnection::disconnect, this));
      }
    });
}

void ClientConnection::doWrite()
{
  assert(isServerThread());

  m_ws->async_write(boost::asio::buffer(**m_writeQueue.front(), m_writeQueue.front()->size()),
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
        EventLoop::call(std::bind(&ClientConnection::disconnect, this));
      }
    });
}

void ClientConnection::processMessage(const std::shared_ptr<Message> message)
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
      m_session = std::make_shared<Session>(std::dynamic_pointer_cast<ClientConnection>(shared_from_this()));
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
    sendMessage(Message::newErrorResponse(message->command(), message->requestId(), LogMessage::C1014_INVALID_COMMAND));
  }
}

void ClientConnection::sendMessage(std::unique_ptr<Message> message)
{
  assert(isEventLoopThread());

  ioContext().post(
    [this, msg=std::make_shared<std::unique_ptr<Message>>(std::move(message))]()
    {
      const bool wasEmpty = m_writeQueue.empty();
      m_writeQueue.emplace(std::move(*msg));
      if(wasEmpty)
        doWrite();
    });
}

void ClientConnection::disconnect()
{
  assert(isEventLoopThread());

  m_session.reset();

  WebSocketConnection::disconnect();
}
