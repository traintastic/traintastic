/**
 * server/src/hardware/commandstation/serialcommandstation.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "serialcommandstation.hpp"
#include "../../core/traintastic.hpp"
#include "../../core/eventloop.hpp"

SerialCommandStation::SerialCommandStation(const std::weak_ptr<World>& world, std::string_view _id) :
  CommandStation(world, _id),
  m_serialPort{Traintastic::instance->ioContext()},
  m_readBufferOffset{0},
  port{this, "port", "/dev/ttyUSB0", PropertyFlags::ReadWrite | PropertyFlags::Store},
  baudrate{this, "baudrate", 19200, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](uint32_t)
    {
      //interface = LocoNetSerialInterface::Custom;
    }},
  flowControl{this, "flow_control", SerialFlowControl::None, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](SerialFlowControl)
    {
      //interface = LocoNetSerialInterface::Custom;
    }}
{
  port.addAttributeEnabled(!online);
  baudrate.addAttributeEnabled(!online);
  flowControl.addAttributeEnabled(!online);

  m_interfaceItems.insertBefore(port, notes);
  m_interfaceItems.insertBefore(baudrate, notes);
  m_interfaceItems.insertBefore(flowControl, notes);
}

bool SerialCommandStation::setOnline(bool& value)
{
  if(!m_serialPort.is_open() && value)
  {
    if(!start())
    {
      value = false;
      return false;
    }
    m_readBufferOffset = 0;
    read();

    //loconet->queryLocoSlots();
  }
  else if(m_serialPort.is_open() && !value)
    stop();

  return true;
}

bool SerialCommandStation::start()
{
  boost::system::error_code ec;
  m_serialPort.open(port, ec);
  if(ec)
  {
    logError("open: " + ec.message());
    return false;
  }

  m_serialPort.set_option(boost::asio::serial_port_base::baud_rate(baudrate));
  m_serialPort.set_option(boost::asio::serial_port_base::character_size(8));
  m_serialPort.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
  m_serialPort.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
  switch(flowControl)
  {
    case SerialFlowControl::None:
      m_serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
      break;

    case SerialFlowControl::Hardware:
      m_serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::hardware));
      break;
  }
  return true;
}

void SerialCommandStation::stop()
{
  // TODO: send power off cmd??
  m_serialPort.close();
}
