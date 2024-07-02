/**
 * server/src/hardware/protocol/marklincan/iohandler/tcpiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023-2024 Reinder Feenstra
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
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../log/logmessageexception.hpp"

namespace MarklinCAN {

TCPIOHandler::TCPIOHandler(Kernel& kernel, std::string hostname)
  : NetworkIOHandler(kernel)
  , m_hostname{std::move(hostname)}
  , m_socket{m_kernel.ioContext()}
  , m_readBufferOffset{0}
{
}

void TCPIOHandler::start()
{
  boost::system::error_code ec;

  m_endpoint.port(port);
  m_endpoint.address(boost::asio::ip::make_address(m_hostname, ec));
  if(ec)
    throw LogMessageException(LogMessage::E2003_MAKE_ADDRESS_FAILED_X, ec);

 m_socket.async_connect(m_endpoint,
    [this](const boost::system::error_code& err)
    {
      if(!err)
      {
        m_socket.set_option(boost::asio::socket_base::linger(true, 0));
        m_socket.set_option(boost::asio::ip::tcp::no_delay(true));

        m_connected = true;

        write();

        NetworkIOHandler::start();
      }
      else if(err != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [this, err]()
          {
            Log::log(m_kernel.logId, LogMessage::E2005_SOCKET_CONNECT_FAILED_X, err);
            m_kernel.error();
          });
      }
    });
}

void TCPIOHandler::stop()
{
  m_socket.cancel();
  m_socket.close();
  m_connected = false;
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
      else if(ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_kernel.logId, LogMessage::E2008_SOCKET_READ_FAILED_X, ec);
            m_kernel.error();
          });
      }
    });
}

void TCPIOHandler::write()
{
  if(m_writeBufferOffset == 0 || !m_connected)
  {
    return;
  }

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
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_kernel.logId, LogMessage::E2007_SOCKET_WRITE_FAILED_X, ec);
            m_kernel.error();
          });
      }
    });
}

}
