/**
 * server/src/hardware/protocol/marklincan/iohandler/tcpiohandler.cpp
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

#include "tcpiohandler.hpp"
#include "../kernel.hpp"
#include "../messages.hpp"
#include "../../../../log/logmessageexception.hpp"

namespace MarklinCAN {

TCPIOHandler::TCPIOHandler(Kernel& kernel, const std::string& hostname)
  : NetworkIOHandler(kernel)
  , m_socket{m_kernel.ioContext()}
  , m_readBufferOffset{0}

{
  boost::system::error_code ec;

  m_endpoint.port(port);
  m_endpoint.address(boost::asio::ip::make_address(hostname, ec));
  if(ec)
    throw LogMessageException(LogMessage::E2003_MAKE_ADDRESS_FAILED_X, ec);

  m_socket.connect(m_endpoint, ec);
  if(ec)
    throw LogMessageException(LogMessage::E2005_SOCKET_CONNECT_FAILED_X, ec);

  m_socket.set_option(boost::asio::socket_base::linger(true, 0));
  m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
}

void TCPIOHandler::stop()
{
  //! \todo implement
}

void TCPIOHandler::read()
{
  m_socket.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        const std::byte* pos = m_readBuffer.data();
        bytesTransferred += m_readBufferOffset;

        while(bytesTransferred >= sizeof(NetworkMessage))
        {
          m_kernel.receive(toMessage(*reinterpret_cast<const NetworkMessage*>(pos)));
          pos += sizeof(NetworkMessage);
          bytesTransferred -= sizeof(NetworkMessage);
        }

        if(bytesTransferred != 0)
          memmove(m_readBuffer.data(), pos, bytesTransferred);
        m_readBufferOffset = bytesTransferred;

        read();
      }
      else{}
        //EventLoop::call(
       //   [this, ec]()
         // {
            //Log::log(*this, LogMessage::E2002_SERIAL_READ_FAILED_X, ec);
            //online = false;
          //});
    });
}

void TCPIOHandler::write()
{
  m_socket.async_write_some(boost::asio::buffer(m_writeBuffer.data(), m_writeBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        if(bytesTransferred < m_writeBufferOffset)
        {
          m_writeBufferOffset -= bytesTransferred;
          memmove(m_writeBuffer.data(), m_writeBuffer.data() + bytesTransferred, m_writeBufferOffset);
          write();
        }
        else
          m_writeBufferOffset = 0;
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        // LogMessage::E1006_SOCKET_WRITE_FAILED_X, ec
      }
    });
}

}
