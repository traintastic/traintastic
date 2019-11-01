/**
 * server/src/core/client.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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
//#include "console.hpp"
//#include "objectregistry.hpp"
#include "traintastic.hpp"
#include "eventloop.hpp"
#include "session.hpp"

Client::Client(Traintastic& server, const std::string& id, boost::asio::ip::tcp::socket socket) :
  m_server{server},
  m_socket(std::move(socket)),
  m_id{id},
  m_authenticated{false}//,
  //m_lastObjectHandle{0}
{
  m_socket.set_option(boost::asio::socket_base::linger(true, 0));
  m_socket.set_option(boost::asio::ip::tcp::no_delay(true));

  Traintastic::instance->console->info(id, "Connected");
}

Client::~Client()
{
  try
  {
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
  }
  catch(...)
  {
  }
  m_socket.close();
}

void Client::start()
{
  doReadHeader();
}

void Client::doReadHeader()
{
  auto self(shared_from_this());
  boost::asio::async_read(m_socket,
    boost::asio::buffer(&m_readBuffer.header, sizeof(m_readBuffer.header)),
    [this, self](const boost::system::error_code& e, std::size_t)
    {
      if(!e)
      {
        m_readBuffer.message.reset(new Message(m_readBuffer.header));
        if(m_readBuffer.message->dataSize() == 0)
        {
          if(m_readBuffer.message->command() != Message::Command::Ping)
            EventLoop::call(&Client::processMessage, this, m_readBuffer.message);
          else
            ; // TODO: ping hier replyen
          m_readBuffer.message.reset();
          doReadHeader();
        }
        else
          doReadData();
      }
      else if(e == boost::asio::error::eof || e == boost::asio::error::connection_aborted || e == boost::asio::error::connection_reset)
        connectionLost();
      else
        connectionError("readheader", e.message());
    });
}

void Client::doReadData()
{
  auto self(shared_from_this());
  boost::asio::async_read(m_socket,
    boost::asio::buffer(m_readBuffer.message->data(), m_readBuffer.message->dataSize()),
      [this, self](const boost::system::error_code& ec, std::size_t)
      {
        if(!ec)
        {
          if(m_readBuffer.message->command() != Message::Command::Ping)
            EventLoop::call(&Client::processMessage, this, m_readBuffer.message);
          else
            ; // TODO: ping hier replyen
          m_readBuffer.message.reset();
          doReadHeader();
        }
        else if(ec == boost::asio::error::eof || ec == boost::asio::error::connection_aborted || ec == boost::asio::error::connection_reset)
          connectionLost();
        else
          connectionError("readdata", ec.message());
      });
}

void Client::doWrite()
{
  auto self(shared_from_this());
  boost::asio::async_write(m_socket, boost::asio::buffer(**m_writeQueue.front(), m_writeQueue.front()->size()),
    [this, self](const boost::system::error_code& ec, std::size_t)
    {
      if(!ec)
      {
        m_writeQueue.pop();
        if(!m_writeQueue.empty())
          doWrite();
      }
      else
        connectionError("write", ec.message());
    });
}

void Client::processMessage(const std::shared_ptr<Message> message)
{
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
      Traintastic::instance->console->notice(m_id, "Created new session");
      auto response = Message::newResponse(message->command(), message->requestId());
      response->write(m_session->uuid());
      sendMessage(std::move(response));
      return;
    }
  }
  else
  {
    if(message->command() == Message::Command::Login && message->type() == Message::Type::Request)
    {
      m_authenticated = true; // oke for now, login can be added later :)
      Traintastic::instance->console->notice(m_id, "Logged in anonymously");
      sendMessage(Message::newResponse(message->command(), message->requestId()));
      return;
    }
  }

  if(message->type() == Message::Type::Request)
  {
    Traintastic::instance->console->debug(m_id, "Received invalid command: " + std::to_string(static_cast<uint8_t>(message->command())));


    usleep(10*1000*1000);


    sendMessage(Message::newErrorResponse(message->command(), message->requestId(), Message::ErrorCode::InvalidCommand));
  }


/*
  switch(message.command())
  {
    case Message::Command::None:
      break;

    case Message::Command::CreateObject:
      if(message.isRequest())
      {
        const std::string id = message.read();
        m_server.console->debug(m_id, std::string("CreateObject: id=") + id);
        //std::unique_ptr<Message> response = Message::newResponse(message.command(), message.requestId());
        //response->add(ObjectRegistry::instance->exists(id));
        //sendMessage(std::move(response));
      }
      break;

    case Message::Command::IsObject:
      if(message.isRequest())
      {
        const std::string id = message.read();
        m_server.console->debug(m_id, std::string("IsObject: id=") + id);
        std::unique_ptr<Message> response = Message::newResponse(message.command(), message.requestId());
        response->add(ObjectRegistry::instance->exists(id));
        sendMessage(std::move(response));
      }
      break;

    case Message::Command::GetObject:
      if(message.isRequest())
      {
        std::string id = message.read();
        m_server.console->debug(m_id, std::string("GetObject: id=") + id);
        ObjectPtr p = ObjectRegistry::instance->get(id);
        if(p)
        {
          ObjectHandle handle = 0;//subscribeObject(p);
          std::unique_ptr<Message> response = Message::newResponse(message.command(), message.requestId());
          response->add(handle);





          sendMessage(std::move(response));
        }
        else
          sendMessage(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::UnknownObjectId, std::string("No object exists with id `") + id + "`"));
      }
      break;

    case Message::Command::ReleaseObject:
      if(message.isRequest())
      {
        ObjectHandle handle = message.read<ObjectHandle>();
        m_server.console->debug(m_id, std::string("ReleaseObject: handle=") + std::to_string(handle));
        if(false)//unsubscribeObject(handle))
          sendMessage(std::move(Message::newResponse(message.command(), message.requestId())));
        else
          sendMessage(std::move(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::InvalidHandle)));
      }
      break;

    case Message::Command::SetProperty:
      if(message.isRequest())
      {
        ObjectHandle handle = message.read<ObjectHandle>();
        std::string name = message.read();
        m_server.console->debug(m_id, std::string("SetProperty: handle=") + std::to_string(handle) + ", name=" + name);
        ObjectPtr object;// = getObject(handle);
        AbstractProperty* property;
        //if(!object)
        //  sendMessage(std::move(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::InvalidHandle)));
        //else if(!(property = object->getProperty(name)))
        //  sendMessage(std::move(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::InvalidProperty)));
        //else if(!readPropertyValue(message, property))
          sendMessage(std::move(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::InvalidValue)));
      }
      break;

    case Message::Command::PropertyChanged:
      assert(false);
      break;
  }
*/
}

void Client::sendMessage(std::unique_ptr<Message> message)
{
  bool wasEmpty = m_writeQueue.empty();
  m_writeQueue.emplace(std::move(message));
  if(wasEmpty)
    doWrite();
}

void Client::connectionLost()
{
  m_server.console->info(m_id, "Connection lost");
  disconnect();
}

void Client::connectionError(const std::string& where, const std::string& what)
{
  m_server.console->error(m_id, what + " [" + where + "]");
  disconnect();
}

void Client::disconnect()
{
  m_server.clientGone(shared_from_this());
}
