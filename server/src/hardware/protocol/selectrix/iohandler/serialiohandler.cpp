/**
 * server/src/hardware/protocol/selectrix/iohandler/serialiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023,2025 Reinder Feenstra
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
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include "../const.hpp"
#include "../kernel.hpp"
#include "../../../../utils/serialport.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"

namespace Selectrix {

SerialIOHandler::SerialIOHandler(Kernel& kernel, const std::string& device, uint32_t baudrate)
  : IOHandler(kernel)
  , m_serialPort{kernel.ioContext()}
{
  SerialPort::open(m_serialPort, device, baudrate, 8, SerialParity::None, SerialStopBits::One, SerialFlowControl::None);
}

SerialIOHandler::~SerialIOHandler()
{
  if(m_serialPort.is_open())
  {
    m_serialPort.close();
  }
}

void SerialIOHandler::start()
{
}

void SerialIOHandler::stop()
{
  m_serialPort.close();
}

bool SerialIOHandler::read(uint8_t address, uint8_t& value)
{
  assert(address <= Address::max);

  boost::system::error_code ec;

  // write read command:
  const std::array<uint8_t, 2> data{address, 0x00};
  boost::asio::write(m_serialPort, boost::asio::buffer(data), ec);
  if(ec)
  {
    EventLoop::call(
      [this, ec]()
      {
        Log::log(m_kernel.logId, LogMessage::E2001_SERIAL_WRITE_FAILED_X, ec);
        m_kernel.error();
      });
    return false;
  }

  // read response:
  boost::asio::read(m_serialPort, boost::asio::buffer(&value, sizeof(value)), ec);
  if(ec)
  {
    EventLoop::call(
      [this, ec]()
      {
        Log::log(m_kernel.logId, LogMessage::E2002_SERIAL_READ_FAILED_X, ec);
        m_kernel.error();
      });
    return false;
  }

  return true;
}

bool SerialIOHandler::write(uint8_t address, uint8_t value)
{
  assert(address <= Address::max);

  boost::system::error_code ec;

  // write command:
  const std::array<uint8_t, 2> data{address |= Address::writeFlag, value};
  boost::asio::write(m_serialPort, boost::asio::buffer(data));
  if(ec)
  {
    EventLoop::call(
      [this, ec]()
      {
        Log::log(m_kernel.logId, LogMessage::E2001_SERIAL_WRITE_FAILED_X, ec);
        m_kernel.error();
      });
    return false;
  }

  return true;
}

}
