/**
 * server/src/hardware/protocol/withrottle/iohandler/tcpiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "tcpiohandler.hpp"
#include "../kernel.hpp"
#include "../../../../log/logmessageexception.hpp"
#include "../../../../log/log.hpp"

namespace WiThrottle {

TCPIOHandler::TCPIOHandler(Kernel& kernel, uint16_t port)
  : IOHandler(kernel)
  , m_port{port}
  , m_acceptor{kernel.ioContext()}
{
}

void TCPIOHandler::start()
{
  boost::system::error_code ec;
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v4::any(), m_port);

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

  doAccept();
}

void TCPIOHandler::stop()
{
  // close socket:
  boost::system::error_code ec;
  m_acceptor.close(ec);

  // disconnect all clients:
  while(!m_clients.empty())
    disconnect(m_clients.begin()->first);
}

bool TCPIOHandler::sendTo(std::string_view message, ClientId clientId)
{
  assert(isKernelThread());

  auto it = m_clients.find(clientId);
  if(it == m_clients.end())
    return false;

  auto& writeBuffer = it->second.writeBuffer;
  const bool writeBufferWasEmpty = writeBuffer.empty();

  writeBuffer.append(message);
  writeBuffer.append("\n");

  if(writeBufferWasEmpty)
    doWrite(clientId);

  return true;
}

bool TCPIOHandler::sendToAll(std::string_view message)
{
  assert(isKernelThread());

  for(auto it : m_clients)
    sendTo(message, it.first);

  return true;
}

void TCPIOHandler::disconnect(ClientId clientId)
{
  assert(isKernelThread());

  auto it = m_clients.find(clientId);
  if(it == m_clients.end())
    return;

  {
    Client& client = it->second;
    boost::system::error_code ec;
    client.socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    client.socket->close(ec);
  }

  m_clients.erase(it);

  m_kernel.clientGone(clientId);
}

void TCPIOHandler::doAccept()
{
  assert(isKernelThread());

  if(!m_socketTCP)
    m_socketTCP = std::make_shared<boost::asio::ip::tcp::socket>(m_kernel.ioContext());

  m_acceptor.async_accept(*m_socketTCP,
    [this](boost::system::error_code ec)
    {
      if(!ec)
      {
        do
        {
          m_lastClientId++;
        }
        while(m_clients.find(m_lastClientId) != m_clients.end() || m_lastClientId == invalidClientId);

        m_clients.emplace(m_lastClientId, Client(std::move(m_socketTCP)));
        doRead(m_lastClientId);
        m_kernel.newClient(m_lastClientId);

        assert(!m_socketTCP);
        m_socketTCP.reset();
        doAccept();
      }
      else
      {
        Log::log(std::string_view{}, LogMessage::E1004_TCP_ACCEPT_ERROR_X, ec.message());
      }
    });
}

void TCPIOHandler::doRead(ClientId clientId)
{
  assert(isKernelThread());

  auto it = m_clients.find(clientId);
  if(it == m_clients.end())
    return;

  Client& client = it->second;

  client.socket->async_read_some(boost::asio::buffer(client.readBuffer.data() + client.readBufferOffset, client.readBuffer.size() - client.readBufferOffset),
    [this, clientId](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        auto it2 = m_clients.find(clientId);
        if(it2 == m_clients.end())
          return;

        Client& src = it2->second;

        const char* pos = reinterpret_cast<const char*>(src.readBuffer.data());
        bytesTransferred += src.readBufferOffset;

        size_t i = 0;
        while(i < bytesTransferred)
        {
          if(*(pos + i) == '\n' || *(pos + i) == '\r')
          {
            if(i > 0)
              m_kernel.receiveFrom(std::string_view{pos, i}, clientId);
            pos += i + 1;
            bytesTransferred -= i + 1;
            i = 0;
          }
          else
            i++;
        }

        if(bytesTransferred != 0)
          memmove(src.readBuffer.data(), pos, bytesTransferred);
        src.readBufferOffset = bytesTransferred;

        doRead(clientId);
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        disconnect(clientId);
      }
    });
}

void TCPIOHandler::doWrite(ClientId clientId)
{
  assert(isKernelThread());

  auto it = m_clients.find(clientId);
  if(it == m_clients.end() || it->second.writeBuffer.empty())
    return;

  Client& client = it->second;

  client.socket->async_write_some(boost::asio::buffer(client.writeBuffer.data(), client.writeBuffer.size()),
    [this, clientId](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        auto it2 = m_clients.find(clientId);
        if(it2 == m_clients.end())
          return;

        auto& writeBuffer = it2->second.writeBuffer;
        writeBuffer.erase(0, bytesTransferred);
        if(!writeBuffer.empty())
          doWrite(clientId);
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        disconnect(clientId);
      }
    });
}

}
