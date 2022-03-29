/**
 * server/src/hardware/protocol/ecos/iohandler/serialiohandler.cpp
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

#include "tcpiohandler.hpp"
#include "../messages.hpp"
#include "../kernel.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../log/logmessageexception.hpp"

namespace ECoS {

TCPIOHandler::TCPIOHandler(Kernel& kernel, const std::string& hostname, uint16_t port)
  : IOHandler(kernel)
  , m_socket{m_kernel.ioContext()}
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

TCPIOHandler::~TCPIOHandler()
{
  if(m_socket.is_open())
    m_socket.close();
}

void TCPIOHandler::start()
{
  read();
}

void TCPIOHandler::stop()
{
  m_socket.close();
}

void TCPIOHandler::read()
{
  m_socket.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        processRead(bytesTransferred);
        read();
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_kernel.logId(), LogMessage::E2008_SOCKET_READ_FAILED_X, ec);
            // TODO interface status -> error
          });
      }
    });
}

void TCPIOHandler::receive(std::string_view message)
{
  IOHandler::receive(message);
  if(m_waitingForReply > 0 && isReply(message))
  {
    m_waitingForReply--;
    if(!m_writing && m_waitingForReply < transferWindow && m_writeBufferOffset > 0)
      write();
  }
}

void TCPIOHandler::write()
{
  assert(!m_writing);

  m_writing = true;

  const char* p = m_writeBuffer.data();
  const char* end = m_writeBuffer.data() + m_writeBufferOffset;

  while(m_waitingForReply < transferWindow && (p = std::find(p, end, '\n')) != end)
  {
    p++;
    m_waitingForReply++;
  }

  if(p == m_writeBuffer.data())
    return;

  m_socket.async_write_some(boost::asio::buffer(m_writeBuffer.data(), p - m_writeBuffer.data()),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      m_writing = false;

      if(!ec)
      {
        if(bytesTransferred < m_writeBufferOffset)
        {
          m_writeBufferOffset -= bytesTransferred;
          memmove(m_writeBuffer.data(), m_writeBuffer.data() + bytesTransferred, m_writeBufferOffset);
          if(m_waitingForReply < transferWindow)
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
            Log::log(m_kernel.logId(), LogMessage::E2007_SOCKET_WRITE_FAILED_X, ec);
            // TODO interface status -> error
          });
      }
    });
}

}
