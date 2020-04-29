/**
 * hardware/commandstation/li10x.cpp
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

#include "li10x.hpp"
#include "../../core/world.hpp"
#include "../../core/traintastic.hpp"
#include "../../core/eventloop.hpp"

namespace Hardware::CommandStation {

LI10x::LI10x(const std::weak_ptr<World>& world, std::string_view _id) :
  CommandStation(world, _id),
  m_serialPort{Traintastic::instance->ioContext()},
  port{this, "port", "", PropertyFlags::ReadWrite},
  baudrate{this, "baudrate", 9600, PropertyFlags::ReadWrite},
  xpressnet{this, "xpressnet", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "LI10x";
  xpressnet.setValueInternal(std::make_shared<::Protocol::XpressNet>(*this, xpressnet.name(), std::bind(&LI10x::send, this, std::placeholders::_1)));

  m_interfaceItems.insertBefore(port, notes);
  m_interfaceItems.insertBefore(baudrate, notes);
  m_interfaceItems.insertBefore(xpressnet, notes);
}

bool LI10x::setOnline(bool& value)
{
  if(!m_serialPort.is_open() && value)
  {
    if(!start())
    {
      value = false;
      return false;
    }
    read();
  }
  else if(m_serialPort.is_open() && !value)
    stop();

  return true;
}

void LI10x::emergencyStopChanged(bool value)
{
  CommandStation::emergencyStopChanged(value);

  if(value)
    send(Protocol::XpressNet::EmergencyStop());
  else if(!trackVoltageOff)
    send(Protocol::XpressNet::NormalOperationResumed());
}

void LI10x::trackVoltageOffChanged(bool value)
{
  CommandStation::trackVoltageOffChanged(value);

  if(!value)
    send(Protocol::XpressNet::NormalOperationResumed());
  else
    send(Protocol::XpressNet::TrackPowerOff());

}

void LI10x::decoderChanged(const Hardware::Decoder& decoder, Hardware::DecoderChangeFlags changes, uint32_t functionNumber)
{
  CommandStation::decoderChanged(decoder, changes, functionNumber);

  if(online)
    xpressnet->decoderChanged(decoder, changes, functionNumber);
}

bool LI10x::start()
{
  boost::system::error_code ec;
  m_serialPort.open(port, ec);
  if(ec)
  {
    Traintastic::instance->console->error(id, "open: " + ec.message());
    return false;
  }

  m_serialPort.set_option(boost::asio::serial_port_base::baud_rate(baudrate));
  m_serialPort.set_option(boost::asio::serial_port_base::character_size(8));
  m_serialPort.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
  m_serialPort.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
  m_serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::hardware));

  return true;
}

void LI10x::stop()
{
  // send bus off cmd
  m_serialPort.close();
}

bool LI10x::send(const Protocol::XpressNet::Message& msg)
{
  assert(Protocol::XpressNet::isChecksumValid(msg));
  if(!m_serialPort.is_open())
    return false;
  boost::system::error_code ec;
  m_serialPort.write_some(boost::asio::buffer(static_cast<const void*>(&msg), msg.size()), ec); // TODO async
  if(ec)
  {
    Traintastic::instance->console->error(id, "write_some: " + ec.message());
    return false;
  }
  return true;
}

void LI10x::receive(std::unique_ptr<uint8_t[]> message)
{
  // NOTE: this runs outside the event loop !!!

  //const uint8_t dataLen = message[0] & 0x0f;

  // TODO: verify checksum

  switch(message[0] & 0xf0)
  {
    case 0x60:
      if(message[1] == 0x01) // Normal operation resumed
      {
        EventLoop::call(
          [this]()
          {
            Traintastic::instance->console->debug(id, "receive: Normal operation resumed");
            emergencyStop.setValueInternal(false);
            trackVoltageOff.setValueInternal(false);
          });
      }
      else if(message[1] == 0x00) // Track power Off
      {
        EventLoop::call(
          [this]()
          {
            Traintastic::instance->console->debug(id, "receive: Track power Off");
            trackVoltageOff.setValueInternal(true);
          });
      }
      else
      {

      }
      break;

    case 0x80:
      if(message[1] == 0x00) //  Emergency Stop
      {
        EventLoop::call(
          [this]()
          {
            Traintastic::instance->console->debug(id, "receive: Emergency Stop");
            emergencyStop.setValueInternal(true);
          });
      }
      else
      {

      }
      break;
  }
}

void LI10x::read()
{
  m_serialPort.async_read_some(boost::asio::buffer(m_readBuffer),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        std::size_t i = 0;
        while(bytesTransferred != 0)
        {
          if(!m_readMessage) // read header byte
          {
            m_readMessageTodo = (m_readBuffer[i] & 0x0f) + 1;
            m_readMessage = std::make_unique<uint8_t[]>(1 + m_readMessageTodo);
            m_readMessage[0] = m_readBuffer[i++];
            m_readMessagePos = 1;
            bytesTransferred--;
          }
          else
          {
            const uint8_t size = static_cast<uint8_t>(std::min<std::size_t>(m_readMessageTodo, bytesTransferred));
            memcpy(m_readMessage.get() + m_readMessagePos, m_readBuffer.data() + i, size);
            i += size;
            if((m_readMessageTodo -= size) == 0)
              receive(std::move(m_readMessage));
            else
              m_readMessagePos += size;
          }
        }
        read();
      }
      else
        EventLoop::call([this, ec](){ Traintastic::instance->console->error(id, "async_read_some: " + ec.message()); });
    });
}

}
