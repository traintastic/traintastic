/**
 * server/src/hardware/protocol/ecos/iohandler/serialiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_IOHANDLER_TCPIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_IOHANDLER_TCPIOHANDLER_HPP

#include "iohandler.hpp"
#include <boost/asio/ip/tcp.hpp>

namespace ECoS {

class TCPIOHandler final : public IOHandler
{
  private:
    static constexpr uint32_t transferWindow = 25;

    const std::string m_hostname;
    const uint16_t m_port;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::endpoint m_endpoint;
    bool m_connected = false;
    std::array<char, 32 * 1024> m_readBuffer;
    size_t m_readBufferOffset = 0;
    size_t m_readPos = 0;
    std::array<char, 32 * 1024> m_writeBuffer;
    size_t m_writeBufferOffset = 0;
    bool m_writing = false;
    uint32_t m_waitingForReply = 0;


    void read();
    void processRead(size_t bytesTransferred);
    void receive(std::string_view message);
    void write();

  public:
    TCPIOHandler(Kernel& kernel, std::string hostname, uint16_t port = 15471);
    ~TCPIOHandler() final;

    void start() final;
    void stop() final;

    bool send(std::string_view message) final;
};

}

#endif

