/**
 * hardware/commandstation/loconettcpbinary.cpp
 *
 * This file is part of the traintastic source code
 *
 * Copyright (C) 2021 Reinder Feenstra <reinderfeenstra@gmail.com>
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

#include "loconettcpbinary.hpp"
#include "../../core/traintastic.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"

LocoNetTCPBinary::LocoNetTCPBinary(const std::weak_ptr<World>& world, std::string_view _id) :
  CommandStation(world, _id),
  m_socket{Traintastic::instance->ioContext()},
  hostname{this, "hostname", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  port{this, "port", 5550, PropertyFlags::ReadWrite | PropertyFlags::Store},
  loconet{this, "loconet", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "LocoNet (TCP binary)";
  loconet.setValueInternal(LocoNet::LocoNet::create(*this, loconet.name(), std::bind(&LocoNetTCPBinary::send, this, std::placeholders::_1)));

  Attributes::addEnabled(hostname, true);
  m_interfaceItems.insertBefore(hostname, notes);
  Attributes::addEnabled(port, true);
  m_interfaceItems.insertBefore(port, notes);
  m_interfaceItems.insertBefore(loconet, notes);
}

bool LocoNetTCPBinary::setOnline(bool& value)
{
  if(!m_socket.is_open() && value)
  {
    boost::system::error_code ec;

    boost::asio::ip::tcp::endpoint endpoint;
    endpoint.port(port);
    endpoint.address(boost::asio::ip::make_address(hostname, ec));
    if(ec)
    {
      Log::log(*this, LogMessage::E2003_MAKE_ADDRESS_FAILED_X, ec);
      return false;
    }

    m_socket.connect(endpoint, ec);
    if(ec)
    {
      Log::log(*this, LogMessage::E2005_SOCKET_CONNECT_FAILED_X, ec);
      return false;
    }

    m_socket.set_option(boost::asio::socket_base::linger(true, 0));
    m_socket.set_option(boost::asio::ip::tcp::no_delay(true));

    receive(); // start receiving messages

    hostname.setAttributeEnabled(false);
    port.setAttributeEnabled(false);
  }
  else if(m_socket.is_open() && !value)
  {
    hostname.setAttributeEnabled(true);
    port.setAttributeEnabled(true);

    m_socket.close();
  }
  return true;
}

void LocoNetTCPBinary::emergencyStopChanged(bool value)
{
  CommandStation::emergencyStopChanged(value);

  if(online)
    loconet->emergencyStopChanged(value);
}

void LocoNetTCPBinary::powerOnChanged(bool value)
{
  CommandStation::powerOnChanged(value);

  if(online)
    loconet->powerOnChanged(value);
}

void LocoNetTCPBinary::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  CommandStation::decoderChanged(decoder, changes, functionNumber);

  if(online)
    loconet->decoderChanged(decoder, changes, functionNumber);
}

void LocoNetTCPBinary::receive()
{
  m_socket.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
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

        receive();
      }
      else
        EventLoop::call(
          [this, ec]()
          {
            Log::log(*this, LogMessage::E2008_SOCKET_READ_FAILED_X, ec);
            online = false;
          });
    });
}

bool LocoNetTCPBinary::send(const LocoNet::Message& message)
{
  if(!m_socket.is_open())
    return false;

  // for now a blocking implementation, will be replaced by async.

  boost::system::error_code ec;
  size_t todo = message.size();
  while(todo > 0)
  {
    todo -= m_socket.write_some(boost::asio::buffer(static_cast<const void*>(&message), message.size()), ec);
    if(ec)
    {
      Log::log(*this, LogMessage::E2007_SOCKET_WRITE_FAILED_X, ec);
      return false;
    }
  }
  return true;
}
