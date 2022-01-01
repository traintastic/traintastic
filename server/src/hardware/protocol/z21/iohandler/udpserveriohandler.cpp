/**
 * server/src/hardware/protocol/z21/iohandler/udpclientiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#include "udpserveriohandler.hpp"
//#include <boost/asio/buffer.hpp>
#include "../serverkernel.hpp"
#include "../messages.hpp"
#include "../../../../log/logmessageexception.hpp"

namespace Z21 {

UDPServerIOHandler::UDPServerIOHandler(ServerKernel& kernel)
  : UDPIOHandler(kernel)
{
  boost::system::error_code ec;

  if(m_socket.open(boost::asio::ip::udp::v4(), ec))
    throw LogMessageException(LogMessage::E2004_SOCKET_OPEN_FAILED_X, ec);

  if(m_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), defaultPort), ec))
  {
    m_socket.close();
    throw LogMessageException(LogMessage::E2006_SOCKET_BIND_FAILED_X, ec);
  }
}

bool UDPServerIOHandler::sendTo(const Message& message, ClientId id)
{
  //! \todo use async_send_to()
  if(auto it = m_clients.find(id); it != m_clients.end())
  {
    m_socket.send_to(boost::asio::buffer(&message, message.dataLen()), it->second);
    return true;
  }

  return false;
}

void UDPServerIOHandler::purgeClient(ClientId id)
{
  m_clients.erase(id);
}

void UDPServerIOHandler::receive(const Message& message, const boost::asio::ip::udp::endpoint& remoteEndpoint)
{
  ClientId clientId;

  if(auto it = std::find_if(m_clients.begin(), m_clients.end(), [&remoteEndpoint](auto n) { return n.second == remoteEndpoint; }); it != m_clients.end())
  {
    clientId = it->first;
  }
  else // new client
  {
    do
    {
      m_lastClientId++;
    }
    while(m_clients.find(m_lastClientId) != m_clients.end());

    m_clients.emplace(m_lastClientId, remoteEndpoint);

    clientId = m_lastClientId;
  }

  static_cast<ServerKernel&>(m_kernel).receiveFrom(message, clientId);
}

}
