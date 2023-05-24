/**
 * server/src/utils/serialport.cpp
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

#include "serialport.hpp"
#include "../log/logmessageexception.hpp"
#ifdef __linux__
  #include "../os/linux/setbaudrate.hpp"
#endif

namespace SerialPort {

void open(boost::asio::serial_port& serialPort, const std::string& device, uint32_t baudrate, uint8_t characterSize, SerialParity parity, SerialStopBits stopBits, SerialFlowControl flowControl)
{
  boost::system::error_code ec;

  serialPort.open(device, ec);
  if(ec)
    throw LogMessageException(LogMessage::E2010_SERIAL_PORT_OPEN_FAILED_X, ec);

  serialPort.set_option(boost::asio::serial_port_base::baud_rate(baudrate), ec);
  if(ec == boost::asio::error::invalid_argument)
  {
#ifdef __linux__
    if(Linux::setBaudrate(serialPort.native_handle(), baudrate))
      ec.clear();
#endif
  }
  if(ec)
    throw LogMessageException(LogMessage::E2013_SERIAL_PORT_SET_BAUDRATE_FAILED_X, ec);

  serialPort.set_option(boost::asio::serial_port_base::character_size(characterSize), ec);
  if(ec)
    throw LogMessageException(LogMessage::E2014_SERIAL_PORT_SET_DATA_BITS_FAILED_X, ec);

  static_cast<void>(parity); // silence unused warning if assertions are off
  assert(parity == SerialParity::None);
  serialPort.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none), ec);
  if(ec)
    throw LogMessageException(LogMessage::E2016_SERIAL_PORT_SET_PARITY_FAILED_X, ec);

  static_cast<void>(stopBits); // silence unused warning if assertions are off
  assert(stopBits == SerialStopBits::One);
  serialPort.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one), ec);
  if(ec)
    throw LogMessageException(LogMessage::E2015_SERIAL_PORT_SET_STOP_BITS_FAILED_X, ec);

  switch(flowControl)
  {
    case SerialFlowControl::None:
      serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
      break;

    case SerialFlowControl::Hardware:
      serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::hardware));
      break;
  }
  if(ec)
    throw LogMessageException(LogMessage::E2017_SERIAL_PORT_SET_FLOW_CONTROL_FAILED_X, ec);
}

}
