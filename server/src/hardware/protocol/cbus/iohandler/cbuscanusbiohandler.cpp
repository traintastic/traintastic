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

#include "cbuscanusbiohandler.hpp"
#include <boost/asio/write.hpp>
#include "../cbuskernel.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../utils/serialport.hpp"

namespace CBUS {

CANUSBIOHandler::CANUSBIOHandler(Kernel& kernel, const std::string& device)
  : ASCIIIOHandler(kernel, canId)
  , m_serialPort{m_kernel.ioContext()}
{
  // FIXME: check serial settings, just a guess, if more settings are needed add them to the interface
  SerialPort::open(m_serialPort, device, 115'200, 8, SerialParity::None, SerialStopBits::One, SerialFlowControl::None);
}

CANUSBIOHandler::~CANUSBIOHandler()
{
  if(m_serialPort.is_open())
  {
    boost::system::error_code ec;
    m_serialPort.close(ec);
    // ignore the error
  }
}

void CANUSBIOHandler::start()
{
  read();
  m_kernel.started();
}

void CANUSBIOHandler::stop()
{
  m_serialPort.close();
}

void CANUSBIOHandler::read()
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
            Log::log(m_kernel.logId, LogMessage::E2002_SERIAL_READ_FAILED_X, ec);
            m_kernel.error();
          });
      }
    });
}

void CANUSBIOHandler::write()
{
  assert(!m_writeQueue.empty());
  const auto& message = m_writeQueue.front();
  boost::asio::async_write(m_serialPort, boost::asio::buffer(message.data(), message.size()),
    [this](const boost::system::error_code& ec, std::size_t /*bytesTransferred*/)
    {
      if(!ec)
      {
        m_writeQueue.pop();
        if(!m_writeQueue.empty())
        {
          write();
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
