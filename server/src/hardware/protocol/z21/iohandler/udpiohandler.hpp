/**
 * server/src/hardware/protocol/z21/iohandler/udpiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_IOHANDLER_UDPIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_IOHANDLER_UDPIOHANDLER_HPP

#include "iohandler.hpp"
#include <boost/asio/ip/udp.hpp>

namespace Z21 {

class UDPIOHandler : public IOHandler
{
  public:
    static constexpr size_t payloadSizeMax = 1500 - 20 - 8; ///< Ethernet MTU - IPv4 header - UDP header

  private:
    boost::asio::ip::udp::endpoint m_receiveEndpoint;
    std::array<std::byte, payloadSizeMax> m_receiveBuffer;

    void receive();

  protected:
    boost::asio::ip::udp::socket m_socket;

    virtual void receive(const Message& message, const boost::asio::ip::udp::endpoint& remoteEndpoint) = 0;

  public:
    static constexpr uint16_t defaultPort = 21105;

    UDPIOHandler(Kernel& kernel);

    void start() override;
    void stop() override;
};

}

#endif
