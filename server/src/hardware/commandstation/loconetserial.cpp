/**
 * hardware/commandstation/loconetserial.cpp
 *
 * This file is part of the traintastic source code
 *
 * Copyright (C) 2019-2020 Reinder Feenstra <reinderfeenstra@gmail.com>
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

#include "loconetserial.hpp"
//#include "../../world/world.hpp"
#include "../../core/traintastic.hpp"
#include "../../core/eventloop.hpp"

using namespace Protocol::LocoNet;

namespace Hardware::CommandStation {

LocoNetSerial::LocoNetSerial(const std::weak_ptr<World>& world, std::string_view _id) :
  CommandStation(world, _id),
  m_serialPort{Traintastic::instance->ioContext()},
  m_readBufferOffset{0},
  port{this, "port", "/dev/ttyUSB0", PropertyFlags::ReadWrite | PropertyFlags::Store},
  interface{this, "interface", LocoNetSerialInterface::Custom, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](LocoNetSerialInterface value)
    {
      switch(value)
      {
        case LocoNetSerialInterface::Custom:
          break;

        case LocoNetSerialInterface::DigiKeijsDR5000:
          baudrate = 115200;
          flowControl = SerialFlowControl::Hardware;
          break;

        case LocoNetSerialInterface::RoSoftLocoNetInterface:
          baudrate = 19200;
          flowControl = SerialFlowControl::Hardware;
          break;
      }
    }},
  baudrate{this, "baudrate", 19200, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](uint32_t)
    {
      interface = LocoNetSerialInterface::Custom;
    }},
  flowControl{this, "flow_control", SerialFlowControl::None, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](SerialFlowControl)
    {
      interface = LocoNetSerialInterface::Custom;
    }},
  loconet{this, "loconet", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "LocoNet (serial)";
  loconet.setValueInternal(std::make_shared<LocoNet>(*this, loconet.name(), std::bind(&LocoNetSerial::send, this, std::placeholders::_1)));

  port.addAttributeEnabled(!online);
  interface.addAttributeEnabled(!online);
  baudrate.addAttributeEnabled(!online);
  flowControl.addAttributeEnabled(!online);

  m_interfaceItems.insertBefore(port, notes);
  m_interfaceItems.insertBefore(interface, notes);
  m_interfaceItems.insertBefore(baudrate, notes);
  m_interfaceItems.insertBefore(flowControl, notes);
  m_interfaceItems.insertBefore(loconet, notes);
}

bool LocoNetSerial::setOnline(bool& value)
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

    loconet->queryLocoSlots();
  }
  else if(m_serialPort.is_open() && !value)
    stop();

  return true;
}

void LocoNetSerial::emergencyStopChanged(bool value)
{
  CommandStation::emergencyStopChanged(value);

  if(online)
    loconet->emergencyStopChanged(value);
}

void LocoNetSerial::trackVoltageOffChanged(bool value)
{
  CommandStation::trackVoltageOffChanged(value);

  if(online)
    loconet->trackVoltageOffChanged(value);
}

void LocoNetSerial::decoderChanged(const Hardware::Decoder& decoder, Hardware::DecoderChangeFlags changes, uint32_t functionNumber)
{
  CommandStation::decoderChanged(decoder, changes, functionNumber);

  if(online)
    loconet->decoderChanged(decoder, changes, functionNumber);
}

bool LocoNetSerial::start()
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

void LocoNetSerial::stop()
{
  // TODO: send power off cmd??
  m_serialPort.close();
}

bool LocoNetSerial::send(const Protocol::LocoNet::Message& message)
{
  if(!m_serialPort.is_open())
    return false;
  boost::system::error_code ec;
  m_serialPort.write_some(boost::asio::buffer(static_cast<const void*>(&message), message.size()), ec); // TODO async
  if(ec)
  {
    logError("write_some: " + ec.message());
    return false;
  }
  return true;
}

void LocoNetSerial::read()
{
  m_serialPort.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        const uint8_t* pos = m_readBuffer.data();
        bytesTransferred += m_readBufferOffset;

        while(bytesTransferred > 1)
        {
          const Protocol::LocoNet::Message* message = reinterpret_cast<const Protocol::LocoNet::Message*>(pos);

          size_t drop = 0;
          while((message->size() == 0 || (message->size() <= bytesTransferred && !Protocol::LocoNet::isValid(*message))) && drop < bytesTransferred)
          {
            drop++;
            pos++;
            bytesTransferred--;
            message = reinterpret_cast<const Protocol::LocoNet::Message*>(pos);
          }

          if(drop != 0)
          {
            EventLoop::call(
              [this, drop]()
              {
                logWarning("received malformed data, dropped " + std::to_string(drop) + " byte(s)");
              });
          }
          else if(message->size() <= bytesTransferred)
          {
            loconet->receive(*message);
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
        EventLoop::call(
          [this, ec]()
          {
            logError("async_read_some: " + ec.message());
            online = false;
          });
    });
}

}
