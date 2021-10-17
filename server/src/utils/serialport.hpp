/**
 * server/src/utils/serialport.hpp
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

#ifndef TRAINTASTIC_SERVER_UTILS_SERIALPORT_HPP
#define TRAINTASTIC_SERVER_UTILS_SERIALPORT_HPP

#include <boost/asio/serial_port.hpp>
#include <traintastic/enum/serialflowcontrol.hpp>

namespace SerialPort {

inline boost::system::error_code setBaudRate(boost::asio::serial_port& serialPort, uint32_t value)
{
  boost::system::error_code ec;
  serialPort.set_option(boost::asio::serial_port_base::baud_rate(value), ec);
  return ec;
}

inline boost::system::error_code setCharacterSize(boost::asio::serial_port& serialPort, uint8_t value)
{
  boost::system::error_code ec;
  serialPort.set_option(boost::asio::serial_port_base::character_size(value), ec);
  return ec;
}

inline boost::system::error_code setStopBitsOne(boost::asio::serial_port& serialPort)
{
  boost::system::error_code ec;
  serialPort.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one), ec);
  return ec;
}

inline boost::system::error_code setParityNone(boost::asio::serial_port& serialPort)
{
  boost::system::error_code ec;
  serialPort.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none), ec);
  return ec;
}

inline boost::system::error_code setFlowControl(boost::asio::serial_port& serialPort, SerialFlowControl flowControl)
{
  boost::system::error_code ec;
  switch(flowControl)
  {
    case SerialFlowControl::None:
      serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
      break;

    case SerialFlowControl::Hardware:
      serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::hardware));
      break;
  }
  return ec;
}

}

#endif
