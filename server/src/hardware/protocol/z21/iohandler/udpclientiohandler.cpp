/**
 * server/src/hardware/protocol/z21/iohandler/udpclientiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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

#include "udpclientiohandler.hpp"
#include "../clientkernel.hpp"
#include "../messages.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../log/logmessageexception.hpp"

namespace Z21 {

UDPClientIOHandler::UDPClientIOHandler(ClientKernel& kernel, const std::string& hostname, uint16_t port)
  : UDPIOHandler(kernel)
  , m_sendBufferOffset{0}
{
  boost::system::error_code ec;

  m_remoteEndpoint.port(port);
  m_remoteEndpoint.address(boost::asio::ip::make_address(hostname, ec));
  if(ec)
    throw LogMessageException(LogMessage::E2003_MAKE_ADDRESS_FAILED_X, ec);

  if(m_socket.open(boost::asio::ip::udp::v4(), ec))
    throw LogMessageException(LogMessage::E2004_SOCKET_OPEN_FAILED_X, ec);

  if(m_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), 0), ec))
  {
    m_socket.close();
    throw LogMessageException(LogMessage::E2006_SOCKET_BIND_FAILED_X, ec);
  }
}

bool UDPClientIOHandler::send(const Message& message)
{
  if(m_sendBufferOffset + message.dataLen() > m_sendBuffer.size())
    return false;

  const bool wasEmpty = m_sendBufferOffset == 0;

  memcpy(m_sendBuffer.data() + m_sendBufferOffset, &message, message.dataLen());
  m_sendBufferOffset += message.dataLen();

  if(wasEmpty)
    send();

  return true;
}

void UDPClientIOHandler::receive(const Message& message, const boost::asio::ip::udp::endpoint& /*remoteEndpoint*/)
{
  static_cast<ClientKernel&>(m_kernel).receive(message);
}

void UDPClientIOHandler::send()
{
  m_socket.async_send_to(boost::asio::buffer(m_sendBuffer.data(), m_sendBufferOffset), m_remoteEndpoint,
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        m_sendBufferOffset -= bytesTransferred;

        if(m_sendBufferOffset > 0)
        {
          memmove(m_sendBuffer.data(), m_sendBuffer.data() + bytesTransferred, m_sendBufferOffset);
          send();
        }
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_kernel.logId, LogMessage::E2011_SOCKET_SEND_FAILED_X, ec);
            // TODO interface status -> error
          });
      }
    });
}

}
