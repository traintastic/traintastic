/**
 * server/src/hardware/protocol/loconet/iohandler/z21iohandler.hpp
 *
 * This file is part of the traintastic source code.
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_IOHANDLER_Z21IOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_IOHANDLER_Z21IOHANDLER_HPP

#include "iohandler.hpp"
#include <boost/asio/ip/udp.hpp>

namespace Z21 {
  struct Message;
}

namespace LocoNet {

class Z21IOHandler final : public IOHandler
{
  private:
    static constexpr size_t payloadSizeMax = 1500 - 20 - 8; ///< Ethernet MTU - IPv4 header - UDP header

    boost::asio::ip::udp::socket m_socket;
    boost::asio::ip::udp::endpoint m_remoteEndpoint;
    boost::asio::ip::udp::endpoint m_receiveEndpoint;
    std::array<std::byte, payloadSizeMax> m_receiveBuffer;
    std::array<std::byte, payloadSizeMax> m_sendBuffer;
    size_t m_sendBufferOffset;

    void receive();
    void send();
    bool send(const Z21::Message& message);

  public:
    Z21IOHandler(Kernel& kernel, const std::string& hostname, uint16_t port = 21105);

    void start() final;
    void stop() final;

    bool send(const Message& message) final;
};

}

#endif
