/**
 * hardware/commandstation/xpressnetserial.cpp
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

#include "xpressnetserial.hpp"
#include "../../world/world.hpp"
#include "../../core/traintastic.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/attributes.hpp"

XpressNetSerial::XpressNetSerial(const std::weak_ptr<World>& world, std::string_view _id) :
  SerialCommandStation(world, _id),
  interface{this, "interface", XpressNetSerialInterface::LenzLI100, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](XpressNetSerialInterface value)
    {
      switch(value)
      {
        case XpressNetSerialInterface::Custom:
          break;

        case XpressNetSerialInterface::LenzLI100:
        case XpressNetSerialInterface::RoSoftS88XPressNetLI:
          baudrate.setValueInternal(9600);
          flowControl.setValueInternal(SerialFlowControl::Hardware);
          break;

        case XpressNetSerialInterface::LenzLI100F:
          baudrate.setValueInternal(19200);
          flowControl.setValueInternal(SerialFlowControl::Hardware);
          break;

        case XpressNetSerialInterface::LenzLI101F:
          baudrate.setValueInternal(19200);
          flowControl.setValueInternal(SerialFlowControl::Hardware);
          break;
      }
    }},
  xpressnet{this, "xpressnet", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "XpressNet (serial)";
  xpressnet.setValueInternal(std::make_shared<XpressNet::XpressNet>(*this, xpressnet.name(), std::bind(&XpressNetSerial::send, this, std::placeholders::_1)));

  Attributes::addValues(interface, XpressNetSerialInterfaceValues);
  m_interfaceItems.insertBefore(interface, baudrate);
  m_interfaceItems.insertBefore(xpressnet, notes);
}

void XpressNetSerial::emergencyStopChanged(bool value)
{
  CommandStation::emergencyStopChanged(value);

  if(online)
    xpressnet->emergencyStopChanged(value);
}

void XpressNetSerial::trackVoltageOffChanged(bool value)
{
  CommandStation::trackVoltageOffChanged(value);

  if(online)
    xpressnet->trackVoltageOffChanged(value);
}

void XpressNetSerial::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  CommandStation::decoderChanged(decoder, changes, functionNumber);

  if(online)
    xpressnet->decoderChanged(decoder, changes, functionNumber);
}

bool XpressNetSerial::send(const XpressNet::Message& msg)
{
  assert(XpressNet::isChecksumValid(msg));
  if(!m_serialPort.is_open())
    return false;
  boost::system::error_code ec;
  m_serialPort.write_some(boost::asio::buffer(static_cast<const void*>(&msg), msg.size()), ec); // TODO async
  if(ec)
  {
    logError("write_some: " + ec.message());
    return false;
  }
  return true;
}

void XpressNetSerial::read()
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
          const XpressNet::Message* message = reinterpret_cast<const XpressNet::Message*>(pos);

          size_t drop = 0;
          while(message->size() <= bytesTransferred && !XpressNet::isChecksumValid(*message) && drop < bytesTransferred)
          {
            drop++;
            pos++;
            bytesTransferred--;
            message = reinterpret_cast<const XpressNet::Message*>(pos);
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
            xpressnet->receive(*message);
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
