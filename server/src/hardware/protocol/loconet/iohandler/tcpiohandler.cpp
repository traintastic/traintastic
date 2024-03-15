/**
 * server/src/hardware/protocol/loconet/iohandler/tcpiohandler.cpp
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

#include "tcpiohandler.hpp"
#include "../kernel.hpp"
#include "../../../../log/logmessageexception.hpp"

namespace LocoNet {

TCPIOHandler::TCPIOHandler(Kernel& kernel, std::string hostname, uint16_t port)
  : IOHandler(kernel)
  , m_hostname{std::move(hostname)}
  , m_port{port}
  , m_socket{m_kernel.ioContext()}
{
}

TCPIOHandler::~TCPIOHandler()
{
}

void TCPIOHandler::start()
{
  boost::system::error_code ec;

  m_endpoint.port(m_port);
  m_endpoint.address(boost::asio::ip::make_address(m_hostname, ec));
  if(ec)
    throw LogMessageException(LogMessage::E2003_MAKE_ADDRESS_FAILED_X, ec);

  m_socket.connect(m_endpoint, ec);
  if(ec)
    throw LogMessageException(LogMessage::E2005_SOCKET_CONNECT_FAILED_X, ec);

  m_socket.set_option(boost::asio::socket_base::linger(true, 0));
  m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
}

}
