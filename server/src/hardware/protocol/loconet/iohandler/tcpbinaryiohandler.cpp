/**
 * server/src/hardware/protocol/loconet/iohandler/tcpbinaryiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#include "tcpbinaryiohandler.hpp"
#include <boost/asio/write.hpp>
#include "../kernel.hpp"
#include "../messages.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"

namespace LocoNet {

TCPBinaryIOHandler::TCPBinaryIOHandler(Kernel& kernel, std::string hostname, uint16_t port)
  : TCPIOHandler(kernel, std::move(hostname), port)
  , m_readBufferOffset{0}
  , m_writeBufferOffset{0}
{
}

void TCPBinaryIOHandler::start()
{
  TCPIOHandler::start();

  read();
}

void TCPBinaryIOHandler::stop()
{
}

bool TCPBinaryIOHandler::send(const Message& message)
{
  if(m_writeBufferOffset + message.size() > m_writeBuffer.size())
    return false;

  const bool wasEmpty = m_writeBufferOffset == 0;
  memcpy(m_writeBuffer.data() + m_writeBufferOffset, &message, message.size());
  m_writeBufferOffset += message.size();

  if(wasEmpty)
    write();

  return true;
}

void TCPBinaryIOHandler::read()
{
  m_socket.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        const std::byte* pos = m_readBuffer.data();
        bytesTransferred += m_readBufferOffset;

        while(bytesTransferred > 1)
        {
          const Message* message = reinterpret_cast<const Message*>(pos);

          size_t drop = 0;
          while((message->size() == 0 || (message->size() <= bytesTransferred && !isValid(*message))) && bytesTransferred > 0)
          {
            drop++;
            pos++;
            bytesTransferred--;
            message = reinterpret_cast<const Message*>(pos);
          }

          if(drop != 0)
          {
            EventLoop::call(
              [this, drop]()
              {
                Log::log(m_kernel.logId, LogMessage::W2001_RECEIVED_MALFORMED_DATA_DROPPED_X_BYTES, drop);
              });
          }
          else if(message->size() <= bytesTransferred)
          {
            m_kernel.receive(*message);
            pos += message->size();
            bytesTransferred -= message->size();
          }
          else
            break;
        }

        if(bytesTransferred != 0)
          memmove(m_readBuffer.data(), pos, bytesTransferred);
        m_readBufferOffset = bytesTransferred;

        read();
      }
      else
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

void TCPBinaryIOHandler::write()
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
