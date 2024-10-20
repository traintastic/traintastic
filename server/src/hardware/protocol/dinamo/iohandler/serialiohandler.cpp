/**
 * server/src/hardware/protocol/dinamo/iohandler/serialiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "serialiohandler.hpp"
#include "../kernel.hpp"
#include "../messages.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../utils/serialport.hpp"
#include "../../../../utils/tohex.hpp"

namespace Dinamo {

SerialIOHandler::SerialIOHandler(Kernel& kernel, const std::string& device)
  : IOHandler(kernel)
  , m_serialPort{m_kernel.ioContext()}
{
  SerialPort::open(m_serialPort, device, 19200, 8, SerialParity::Odd, SerialStopBits::One, SerialFlowControl::None);
}

SerialIOHandler::~SerialIOHandler()
{
  if(m_serialPort.is_open())
  {
    boost::system::error_code ec;
    m_serialPort.close(ec);
    // ignore the error
  }
}

void SerialIOHandler::start()
{
  read();
  m_kernel.started();
}

void SerialIOHandler::stop()
{
  m_serialPort.close();
}

bool SerialIOHandler::send(const Message& message)
{
  if(m_writeBufferOffset + message.size() > m_writeBuffer.size())
  {
    return false;
  }

  const bool wasEmpty = m_writeBufferOffset == 0;

  memcpy(m_writeBuffer.data() + m_writeBufferOffset, &message, message.size());
  m_writeBufferOffset += message.size();

  if(wasEmpty)
  {
    write();
  }

  return true;
}

void SerialIOHandler::read()
{
  m_serialPort.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        const std::byte* pos = m_readBuffer.data();
        bytesTransferred += m_readBufferOffset;

        while(bytesTransferred > 1)
        {
          const Message* message = nullptr;
          size_t drop = 0;

          while(drop < bytesTransferred)
          {
            message = reinterpret_cast<const Message*>(pos);
            if(message->size() > bytesTransferred || isValid(*message))
            {
              break;
            }

            drop++;
            pos++;
            bytesTransferred--;
          }

          if(drop != 0)
          {
            EventLoop::call(
              [this, drop, bytes=toHex(pos - drop, drop, true)]()
              {
                Log::log(m_kernel.logId, LogMessage::W2003_RECEIVED_MALFORMED_DATA_DROPPED_X_BYTES_X, drop, bytes);
              });
          }

          assert(message);
          if(message->size() <= bytesTransferred)
          {
            m_kernel.receive(*message);
            pos += message->size();
            bytesTransferred -= message->size();
          }
          else
          {
            break;
          }
        }

        if(bytesTransferred != 0)
        {
          memmove(m_readBuffer.data(), pos, bytesTransferred);
        }
        m_readBufferOffset = bytesTransferred;

        read();
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_kernel.logId, LogMessage::E2002_SERIAL_READ_FAILED_X, ec);
            m_kernel.error();
          });
      }
    });
}

void SerialIOHandler::write()
{
  m_serialPort.async_write_some(boost::asio::buffer(m_writeBuffer.data(), m_writeBufferOffset),
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
            Log::log(m_kernel.logId, LogMessage::E2001_SERIAL_WRITE_FAILED_X, ec);
            m_kernel.error();
          });
      }
    });
}

}
