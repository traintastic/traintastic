/**
 * server/src/hardware/commandstation/dccplusplusserial.cpp
 *
 * This file is part of the traintastic source code
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

#include "dccplusplusserial.hpp"
#include "../protocol/dccplusplus/messages.hpp"
#include "../../core/eventloop.hpp"
#include "../../log/log.hpp"

DCCPlusPlusSerial::DCCPlusPlusSerial(const std::weak_ptr<World>& world, std::string_view _id) :
  SerialCommandStation(world, _id),
  dccPlusPlus{this, "dcc_plus_plus", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "DCC++ (serial)";
  baudrate = 115200;
  dccPlusPlus.setValueInternal(std::make_shared<DCCPlusPlus::DCCPlusPlus>(*this, dccPlusPlus.name(), std::bind(&DCCPlusPlusSerial::send, this, std::placeholders::_1)));

  m_interfaceItems.insertBefore(dccPlusPlus, notes);
}

void DCCPlusPlusSerial::emergencyStopChanged(bool value)
{
  CommandStation::emergencyStopChanged(value);

  if(online)
    dccPlusPlus->emergencyStopChanged(value);
}

void DCCPlusPlusSerial::powerOnChanged(bool value)
{
  CommandStation::powerOnChanged(value);

  if(online)
    dccPlusPlus->powerOnChanged(value);
}

void DCCPlusPlusSerial::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  CommandStation::decoderChanged(decoder, changes, functionNumber);

  if(online)
    dccPlusPlus->decoderChanged(decoder, changes, functionNumber);
}

bool DCCPlusPlusSerial::send(std::string_view message)
{
  if(!m_serialPort.is_open())
    return false;

  // for now a blocking implementation, will be replaced by async.

  boost::system::error_code ec;
  size_t todo = message.size();
  while(todo > 0)
  {
    todo -= m_serialPort.write_some(boost::asio::buffer(message.data(), message.size()), ec);
    if(ec)
    {
      Log::log(*this, LogMessage::E2001_SERIAL_WRITE_FAILED_X, ec);
      return false;
    }
  }
  return true;
}

void DCCPlusPlusSerial::started()
{
  send(DCCPlusPlus::Ex::setSpeedSteps(dccPlusPlus->speedSteps));
}

void DCCPlusPlusSerial::read()
{
  m_serialPort.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        const char* pos = reinterpret_cast<const char*>(m_readBuffer.data());
        bytesTransferred += m_readBufferOffset;

        size_t i = 0;
        while(i < bytesTransferred)
        {
          if(*(pos + i) == '\n')
          {
            dccPlusPlus->receive(std::string_view{pos, i + 1});
            pos += i + 1;
            bytesTransferred -= i + 1;
            i = 0;
          }
          else
            i++;
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
