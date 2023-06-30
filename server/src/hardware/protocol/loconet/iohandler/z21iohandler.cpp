/**
 * server/src/hardware/protocol/loconet/iohandler/z21iohandler.cpp
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

#include "z21iohandler.hpp"
#include <boost/asio/write.hpp>
#include "../kernel.hpp"
#include "../messages.hpp"
#include "../../z21/messages.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../log/logmessageexception.hpp"

namespace LocoNet {

Z21IOHandler::Z21IOHandler(Kernel& kernel, const std::string& hostname, uint16_t port)
  : IOHandler(kernel)
  , m_socket{m_kernel.ioContext()}
  , m_sendBufferOffset{0}
{
  boost::system::error_code ec;

  m_remoteEndpoint.port(port);
  m_remoteEndpoint.address(boost::asio::ip::make_address(hostname, ec));
  if(ec)
    throw LogMessageException(LogMessage::E2003_MAKE_ADDRESS_FAILED_X, ec);

  if(m_socket.open(boost::asio::ip::udp::v4(), ec))
    throw LogMessageException(LogMessage::E2004_SOCKET_OPEN_FAILED_X, ec);

  if(m_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), 0), ec))
  {
    m_socket.close();
    throw LogMessageException(LogMessage::E2006_SOCKET_BIND_FAILED_X, ec);
  }
}

void Z21IOHandler::start()
{
  receive();

  send(Z21::LanSetBroadcastFlags(Z21::BroadcastFlags::LocoNet));
}

void Z21IOHandler::stop()
{
  send(Z21::LanLogoff());

  m_socket.close();
}

bool Z21IOHandler::send(const Message& message)
{
  const uint16_t dataLen = sizeof(Z21::Message) + message.size();

  if(m_sendBufferOffset + dataLen > m_sendBuffer.size())
    return false;

  const bool wasEmpty = m_sendBufferOffset == 0;

  // Z21 header:
  const Z21::Message header{dataLen, Z21::LAN_LOCONET_FROM_LAN};
  memcpy(m_sendBuffer.data() + m_sendBufferOffset, &header, sizeof(header));
  m_sendBufferOffset += sizeof(header);

  // LocoNet message:
  memcpy(m_sendBuffer.data() + m_sendBufferOffset, &message, message.size());
  m_sendBufferOffset += message.size();

  if(wasEmpty)
    send();

  return true;
}

bool Z21IOHandler::send(const Z21::Message& message)
{
  if(m_sendBufferOffset + message.dataLen() > m_sendBuffer.size())
    return false;

  const bool wasEmpty = m_sendBufferOffset == 0;

  memcpy(m_sendBuffer.data() + m_sendBufferOffset, &message, message.dataLen());
  m_sendBufferOffset += message.dataLen();

  if(wasEmpty)
    send();

  return true;
}

void Z21IOHandler::receive()
{
  m_socket.async_receive_from(boost::asio::buffer(m_receiveBuffer), m_receiveEndpoint,
    [this](const boost::system::error_code& ec, std::size_t bytesReceived)
    {
      if(!ec)
      {
        const std::byte* pos = m_receiveBuffer.data();
        while(bytesReceived >= sizeof(Message))
        {
          const Z21::Message& message = *reinterpret_cast<const Z21::Message*>(pos);
          switch(message.header())
          {
            case Z21::LAN_LOCONET_Z21_RX:
            case Z21::LAN_LOCONET_Z21_TX:
            case Z21::LAN_LOCONET_FROM_LAN:
            {
              const Message* msg = reinterpret_cast<const Message*>(pos + sizeof(Z21::Message));
              if(isValid(*msg))
                m_kernel.receive(*msg);
              break;
            }
            default:
              break;
          }

          pos += message.dataLen();
          bytesReceived -= message.dataLen();
        }
        receive();
      }
      else
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_kernel.logId(), LogMessage::E2009_SOCKET_RECEIVE_FAILED_X, ec);
            m_kernel.error();
          });
      }
    });
}

void Z21IOHandler::send()
{
  m_socket.async_send_to(boost::asio::buffer(m_sendBuffer.data(), m_sendBufferOffset), m_remoteEndpoint,
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        // echo back to kernel, kernel expects echo but Z21 doesn't send it:
        const std::byte* pos = m_sendBuffer.data();
        const std::byte* end = pos + bytesTransferred;
        while(pos < end)
        {
          if(reinterpret_cast<const Z21::Message*>(pos)->header() == Z21::LAN_LOCONET_FROM_LAN)
            m_kernel.receive(*reinterpret_cast<const Message*>(pos + sizeof(Z21::Message)));
          pos += reinterpret_cast<const Z21::Message*>(pos)->dataLen();
        }

        m_sendBufferOffset -= bytesTransferred;

        if(m_sendBufferOffset > 0)
        {
          memmove(m_sendBuffer.data(), m_sendBuffer.data() + bytesTransferred, m_sendBufferOffset);
          send();
        }
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        EventLoop::call(
          [this, ec]()
          {
            Log::log(m_kernel.logId(), LogMessage::E2011_SOCKET_SEND_FAILED_X, ec);
            m_kernel.error();
          });
      }
    });
}

}
