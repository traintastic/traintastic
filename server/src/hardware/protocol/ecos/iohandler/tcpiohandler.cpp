/**
 * server/src/hardware/protocol/ecos/iohandler/serialiohandler.cpp
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

#include "tcpiohandler.hpp"
#include "../messages.hpp"
#include "../kernel.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../log/logmessageexception.hpp"

namespace ECoS {

TCPIOHandler::TCPIOHandler(Kernel& kernel, std::string hostname, uint16_t port)
  : IOHandler(kernel)
  , m_hostname{std::move(hostname)}
  , m_port{port}
  , m_socket{m_kernel.ioContext()}
{
}

TCPIOHandler::~TCPIOHandler()
{
  if(m_socket.is_open())
    m_socket.close();
}

void TCPIOHandler::start()
{
  boost::system::error_code ec;

  m_endpoint.port(m_port);
  m_endpoint.address(boost::asio::ip::make_address(m_hostname, ec));
  if(ec)
    throw LogMessageException(LogMessage::E2003_MAKE_ADDRESS_FAILED_X, ec);

  m_socket.connect(m_endpoint, ec);
  if(ec)
    throw LogMessageException(LogMessage::E2005_SOCKET_CONNECT_FAILED_X, ec);


  m_socket.set_option(boost::asio::socket_base::linger(true, 0));
  m_socket.set_option(boost::asio::ip::tcp::no_delay(true));

  read();
}

void TCPIOHandler::stop()
{
  m_socket.close();
}

bool TCPIOHandler::send(std::string_view message)
{
  if(m_writeBufferOffset + message.size() > m_writeBuffer.size())
    return false;

  const bool wasEmpty = m_writeBufferOffset == 0;
  memcpy(m_writeBuffer.data() + m_writeBufferOffset, message.data(), message.size());
  m_writeBufferOffset += message.size();

  if(wasEmpty)
    write();

  return true;
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
            Log::log(m_kernel.logId, LogMessage::E2008_SOCKET_READ_FAILED_X, ec);
            m_kernel.error();
          });
      }
    });
}

void TCPIOHandler::processRead(size_t bytesTransferred)
{
  static constexpr std::string_view typeReply{"REPLY"};
  static constexpr std::string_view typeEvent{"EVENT"};
  static constexpr size_t typeLength = 5;

  std::string_view buffer{m_readBuffer.data(), m_readBufferOffset + bytesTransferred};

  //! @todo this can be a bit optimized by remembering the "state" when a message in not yet complete.
  while(m_readPos != buffer.size())
  {
    m_readPos = buffer.find('<', m_readPos);
    if(m_readPos != std::string_view::npos)
    {
      if((buffer.size() - m_readPos) >= typeLength)
      {
        std::string_view type{m_readBuffer.data() + m_readPos + 1, typeLength};
        if(type == typeReply || type == typeEvent)
        {
          size_t pos = buffer.find(std::string_view{"<END"}, m_readPos);
          if(pos != std::string_view::npos)
          {
            size_t end = buffer.find('>', pos);
            if(end != std::string_view::npos)
            {
              receive(std::string_view{m_readBuffer.data() + m_readPos, end - m_readPos + 1});
              m_readPos = end + 1;
            }
            else
              break;
          }
          else
            break;
        }
      }
      else
        break;
    }
    else
      m_readPos = buffer.size();
  }

  if(m_readPos > 0)
  {
    assert(m_readPos <= buffer.size());
    m_readBufferOffset = buffer.size() - m_readPos;
    if(m_readBufferOffset > 0)
      memmove(m_readBuffer.data(), m_readBuffer.data() + m_readPos, m_readBufferOffset);
    m_readPos = 0;
  }
}

void TCPIOHandler::receive(std::string_view message)
{
  m_kernel.receive(message);

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
            Log::log(m_kernel.logId, LogMessage::E2007_SOCKET_WRITE_FAILED_X, ec);
            m_kernel.error();
          });
      }
    });
}

}
