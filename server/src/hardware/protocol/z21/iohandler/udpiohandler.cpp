/**
 * server/src/hardware/protocol/z21/iohandler/udpiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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

#include "udpiohandler.hpp"
#include "../kernel.hpp"
#include "../messages.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"
#include "../../../../log/logmessageexception.hpp"

namespace Z21 {

UDPIOHandler::UDPIOHandler(Kernel& kernel)
  : IOHandler(kernel)
  , m_socket{m_kernel.ioContext()}
{
}

void UDPIOHandler::start()
{
  receive();
}

void UDPIOHandler::stop()
{
}

void UDPIOHandler::receive()
{
  m_socket.async_receive_from(boost::asio::buffer(m_receiveBuffer), m_receiveEndpoint,
    [this](const boost::system::error_code& ec, std::size_t bytesReceived)
    {
      if(!ec)
      {
        const std::byte* pos = m_receiveBuffer.data();
        while(bytesReceived >= sizeof(Message))
        {
          const Message& message = *reinterpret_cast<const Message*>(pos);
          if(message.dataLen() < sizeof(Message))
            break;
          receive(message, m_receiveEndpoint);
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
            Log::log(m_kernel.logId, LogMessage::E2009_SOCKET_RECEIVE_FAILED_X, ec);
          });
      }
    });
}

}
