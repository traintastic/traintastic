/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "rclinkserialiohandler.hpp"
#include <boost/asio/write.hpp>
#include "../rclinkerror.hpp"
#include "../rclinkkernel.hpp"
#include "../rclinkmessages.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../utils/bit.hpp"
#include "../../../../utils/contains.hpp"
#include "../../../../utils/serialport.hpp"

namespace RCLink {

namespace
{
  constexpr uint8_t messageTerminator = 0xFF;

  constexpr std::array<Command, 2> startBytes{{
    Command::Detector,
    Command::Info,
  }};

  constexpr size_t getMessageSize(Command command)
  {
    switch(command)
    {
      case Command::Detector:
        return sizeof(Detector);

      case Command::Info:
        return sizeof(Info);

      default:
        break;
    }
    return sizeof(Message); // just the command, we should not get here
  }
}

SerialIOHandler::SerialIOHandler(Kernel& kernel, const std::string& device)
  : IOHandler(kernel)
  , m_serialPort{m_kernel.ioContext()}
{
  SerialPort::open(m_serialPort, device, 19'200, 8, SerialParity::None, SerialStopBits::One, SerialFlowControl::Hardware);
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
  IOHandler::start();
}

void SerialIOHandler::stop()
{
  m_serialPort.close();
}

std::error_code SerialIOHandler::send(std::span<const uint8_t> message)
{
  if(m_txBufferOffset + message.size() > m_txBuffer.size())
  {
    return Error::bufferFull();
  }

  const bool wasEmpty = m_txBufferOffset == 0;
  memcpy(m_txBuffer.data() + m_txBufferOffset, message.data(), message.size());
  m_txBufferOffset += message.size();

  if(wasEmpty)
  {
    write();
  }

  return {};
}

void SerialIOHandler::read()
{
  m_serialPort.async_read_some(boost::asio::buffer(m_rxBuffer.data() + m_rxBufferOffset, m_rxBuffer.size() - m_rxBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        const auto* pos = m_rxBuffer.data();
        bytesTransferred += m_rxBufferOffset;

        while(bytesTransferred > 1)
        {
          const auto* message = reinterpret_cast<const Message*>(pos);

          size_t drop = 0;
          while(!contains(startBytes, message->command) && bytesTransferred > 0)
          {
            drop++;
            pos++;
            bytesTransferred--;
            message = reinterpret_cast<const Message*>(pos);
          }

          const size_t messageSize = (bytesTransferred > 0) ? getMessageSize(message->command) : 0;

          if(bytesTransferred == 0 ||
              messageSize + sizeof(messageTerminator) > bytesTransferred ||
              pos[messageSize] != messageTerminator)
          {
            drop++;
            pos++;
            bytesTransferred--;
            message = nullptr;
          }

          if(drop != 0)
          {
            EventLoop::call(
              [this, drop]()
              {
                Log::log(m_kernel.logId, LogMessage::W2001_RECEIVED_MALFORMED_DATA_DROPPED_X_BYTES, drop);
              });
          }

          if(message)
          {
            m_kernel.receive({pos, messageSize});
            pos += messageSize + sizeof(messageTerminator);
            bytesTransferred -= messageSize + sizeof(messageTerminator);
          }
          else
            break;
        }

        if(bytesTransferred != 0)
        {
          memmove(m_rxBuffer.data(), pos, bytesTransferred);
        }
        m_rxBufferOffset = bytesTransferred;

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
  m_serialPort.async_write_some(boost::asio::buffer(m_txBuffer.data(), m_txBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        if(bytesTransferred < m_txBufferOffset)
        {
          m_txBufferOffset -= bytesTransferred;
          memmove(m_txBuffer.data(), m_txBuffer.data() + bytesTransferred, m_txBufferOffset);
          write();
        }
        else
        {
          m_txBufferOffset = 0;
        }
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
