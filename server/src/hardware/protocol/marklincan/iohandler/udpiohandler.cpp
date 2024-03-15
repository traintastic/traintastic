/**
 * server/src/hardware/protocol/marklincan/iohandler/udpiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "udpiohandler.hpp"
#include "../kernel.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../log/logmessageexception.hpp"

namespace MarklinCAN {

UDPIOHandler::UDPIOHandler(Kernel& kernel, const std::string& hostname)
  : NetworkIOHandler(kernel)
  , m_readSocket{m_kernel.ioContext()}
  , m_writeSocket{m_kernel.ioContext()}
{
  boost::system::error_code ec;

  // read:
  if(m_readSocket.open(boost::asio::ip::udp::v4(), ec))
    throw LogMessageException(LogMessage::E2004_SOCKET_OPEN_FAILED_X, ec);

  m_readSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));

  if(m_readSocket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), localPort), ec))
  {
    m_readSocket.close();
    throw LogMessageException(LogMessage::E2006_SOCKET_BIND_FAILED_X, ec);
  }

  // write:
  m_writeEndpoint.port(remotePort);
  m_writeEndpoint.address(boost::asio::ip::make_address(hostname, ec));
  if(ec)
    throw LogMessageException(LogMessage::E2003_MAKE_ADDRESS_FAILED_X, ec);

  if(m_writeSocket.open(boost::asio::ip::udp::v4(), ec))
    throw LogMessageException(LogMessage::E2004_SOCKET_OPEN_FAILED_X, ec);

  m_writeSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));

  if(m_writeSocket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), localPort), ec))
  {
    m_writeSocket.close();
    throw LogMessageException(LogMessage::E2006_SOCKET_BIND_FAILED_X, ec);
  }
}

void UDPIOHandler::stop()
{
  m_readSocket.cancel();
  m_writeSocket.cancel();
  m_readSocket.close();
  m_writeSocket.close();
}

void UDPIOHandler::read()
{
  m_readSocket.async_receive_from(boost::asio::buffer(m_readBuffer), m_readEndpoint,
    [this](const boost::system::error_code& ec, std::size_t bytesReceived)
    {
      if(!ec)
      {
        if(bytesReceived == sizeof(NetworkMessage))
        {
          m_kernel.receive(toMessage(*reinterpret_cast<const NetworkMessage*>(m_readBuffer.data())));
        }

        read();
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_kernel.logId, LogMessage::E2009_SOCKET_RECEIVE_FAILED_X, ec);
            m_kernel.error();
          });
      }
    });
}

void UDPIOHandler::write()
{
  assert(m_writeBufferOffset >= sizeof(NetworkMessage));
  assert(m_writeBufferOffset % sizeof(NetworkMessage) == 0);

  m_writeSocket.async_send_to(boost::asio::buffer(m_writeBuffer.data(), sizeof(NetworkMessage)), m_writeEndpoint,
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        assert(bytesTransferred == sizeof(NetworkMessage));

        m_writeBufferOffset -= bytesTransferred;

        if(m_writeBufferOffset > 0)
        {
          memmove(m_writeBuffer.data(), m_writeBuffer.data() + bytesTransferred, m_writeBufferOffset);
          write();
        }
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_kernel.logId, LogMessage::E2011_SOCKET_SEND_FAILED_X, ec);
            m_kernel.error();
          });
      }
    });
}

}
