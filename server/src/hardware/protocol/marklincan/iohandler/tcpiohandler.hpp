/**
 * server/src/hardware/protocol/marklincan/iohandler/tcpiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_IOHANDLER_TCPIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_IOHANDLER_TCPIOHANDLER_HPP

#include "networkiohandler.hpp"
#include <boost/asio/ip/tcp.hpp>

namespace MarklinCAN {

class TCPIOHandler final : public NetworkIOHandler
{
  private:
    static constexpr uint16_t port = 15731;

    const std::string m_hostname;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::endpoint m_endpoint;
    std::array<std::byte, 1500> m_readBuffer;
    size_t m_readBufferOffset;
    bool m_connected = false;

    void read() final;
    void write() final;

  public:
    TCPIOHandler(Kernel& kernel, std::string hostname);

    void start() final;
    void stop() final;
};

}

#endif
