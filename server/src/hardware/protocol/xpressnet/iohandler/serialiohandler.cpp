/**
 * server/src/hardware/protocol/xpressnet/iohandler/serialiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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
#include "../../../../log/logmessageexception.hpp"
#include "../../../../utils/serialport.hpp"

namespace XpressNet {

SerialIOHandler::SerialIOHandler(Kernel& kernel, const std::string& device, uint32_t baudrate, SerialFlowControl flowControl)
  : IOHandler(kernel)
  , m_serialPort{m_kernel.ioContext()}
{
  using namespace SerialPort;

  boost::system::error_code ec;

  m_serialPort.open(device, ec);
  if(ec)
    throw LogMessageException(LogMessage::E2010_SERIAL_PORT_OPEN_FAILED_X, ec);

  if((ec = setBaudRate(m_serialPort, baudrate)))
    throw LogMessageException(LogMessage::E2013_SERIAL_PORT_SET_BAUDRATE_FAILED_X, ec);

  if((ec = setCharacterSize(m_serialPort, characterSize)))
    throw LogMessageException(LogMessage::E2014_SERIAL_PORT_SET_DATA_BITS_FAILED_X, ec);

  if((ec = setStopBitsOne(m_serialPort)))
    throw LogMessageException(LogMessage::E2015_SERIAL_PORT_SET_STOP_BITS_FAILED_X, ec);

  if((ec = setParityNone(m_serialPort)))
    throw LogMessageException(LogMessage::E2016_SERIAL_PORT_SET_PARITY_FAILED_X, ec);

  if((ec = setFlowControl(m_serialPort, flowControl)))
    throw LogMessageException(LogMessage::E2017_SERIAL_PORT_SET_FLOW_CONTROL_FAILED_X, ec);
}

SerialIOHandler::~SerialIOHandler()
{
  if(m_serialPort.is_open())
    m_serialPort.close();
}

void SerialIOHandler::start()
{
  read();
}

void SerialIOHandler::stop()
{
  m_serialPort.close();
}

void SerialIOHandler::read()
{
  m_serialPort.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
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
            Log::log(m_kernel.logId(), LogMessage::E2002_SERIAL_READ_FAILED_X, ec);
            // TODO interface status -> error
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
            Log::log(m_kernel.logId(), LogMessage::E2001_SERIAL_WRITE_FAILED_X, ec);
            // TODO interface status -> error
          });
      }
    });
}

}
