/**
 * hardware/commandstation/loconetserial.cpp
 *
 * This file is part of the traintastic source code
 *
 * Copyright (C) 2019-2021 Reinder Feenstra <reinderfeenstra@gmail.com>
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
#include "../../core/traintastic.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"
#include "../../utils/displayname.hpp"

LocoNetSerial::LocoNetSerial(const std::weak_ptr<World>& world, std::string_view _id) :
  SerialCommandStation(world, _id),
  interface{this, "interface", LocoNetSerialInterface::Custom, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](LocoNetSerialInterface value)
    {
      switch(value)
      {
        case LocoNetSerialInterface::Custom:
          break;

        case LocoNetSerialInterface::DigikeijsDR5000:
          baudrate = 115200;
          flowControl = SerialFlowControl::Hardware;
          break;

        case LocoNetSerialInterface::RoSoftLocoNetInterface:
          baudrate = 19200;
          flowControl = SerialFlowControl::Hardware;
          break;
      }
    }},
  loconet{this, "loconet", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "LocoNet (serial)";
  loconet.setValueInternal(LocoNet::LocoNet::create(*this, loconet.name(), std::bind(&LocoNetSerial::send, this, std::placeholders::_1)));

  Attributes::addEnabled(interface, !online);
  Attributes::addValues(interface, LocoNetSerialInterfaceValues);
  m_interfaceItems.insertBefore(interface, baudrate);

  Attributes::addDisplayName(loconet, DisplayName::Hardware::loconet);
  m_interfaceItems.insertBefore(loconet, notes);
}

void LocoNetSerial::emergencyStopChanged(bool value)
{
  CommandStation::emergencyStopChanged(value);

  if(online)
    loconet->emergencyStopChanged(value);
}

void LocoNetSerial::powerOnChanged(bool value)
{
  CommandStation::powerOnChanged(value);

  if(online)
    loconet->powerOnChanged(value);
}

void LocoNetSerial::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  CommandStation::decoderChanged(decoder, changes, functionNumber);

  if(online)
    loconet->decoderChanged(decoder, changes, functionNumber);
}

bool LocoNetSerial::send(const LocoNet::Message& message)
{
  if(!m_serialPort.is_open())
    return false;
  boost::system::error_code ec;
  m_serialPort.write_some(boost::asio::buffer(static_cast<const void*>(&message), message.size()), ec); // TODO async
  if(ec)
  {
    Log::log(*this, LogMessage::E2001_SERIAL_WRITE_FAILED_X, ec);
    return false;
  }
  return true;
}

void LocoNetSerial::started()
{
  loconet->queryLocoSlots();
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
          const LocoNet::Message* message = reinterpret_cast<const LocoNet::Message*>(pos);

          size_t drop = 0;
          while((message->size() == 0 || (message->size() <= bytesTransferred && !LocoNet::isValid(*message))) && drop < bytesTransferred)
          {
            drop++;
            pos++;
            bytesTransferred--;
            message = reinterpret_cast<const LocoNet::Message*>(pos);
          }

          if(drop != 0)
          {
            EventLoop::call(
              [this, drop]()
              {
                Log::log(*this, LogMessage::W2001_RECEIVED_MALFORMED_DATA_DROPPED_X_BYTES, drop);
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
            Log::log(*this, LogMessage::E2002_SERIAL_READ_FAILED_X, ec);
            online = false;
          });
    });
}
