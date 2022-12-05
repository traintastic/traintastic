/**
 * server/src/hardware/protocol/withrottle/iohandler/tcpiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_WITHROTTLE_IOHANDLER_TCPIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_WITHROTTLE_IOHANDLER_TCPIOHANDLER_HPP

#include "iohandler.hpp"
#include <array>
#include <unordered_map>
#include <boost/asio/ip/tcp.hpp>

namespace WiThrottle {

class TCPIOHandler : public IOHandler
{
  private:
    struct Client
    {
      std::shared_ptr<boost::asio::ip::tcp::socket> socket;
      std::array<char, 4096> readBuffer;
      size_t readBufferOffset = 0;
      std::string writeBuffer;

      Client(std::shared_ptr<boost::asio::ip::tcp::socket> socket_)
        : socket{std::move(socket_)}
      {
      }
    };

    const uint16_t m_port;
    boost::asio::ip::tcp::acceptor m_acceptor;
    std::shared_ptr<boost::asio::ip::tcp::socket> m_socketTCP;
    ClientId m_lastClientId = 0;
    std::unordered_map<ClientId, Client> m_clients;

    void doAccept();
    void doRead(ClientId clientId);
    void doWrite(ClientId clientId);

  public:
    TCPIOHandler(Kernel& kernel, uint16_t port);

    bool hasClients() const final { return !m_clients.empty(); }

    void start() override;
    void stop() override;

    bool sendTo(std::string_view message, ClientId clientId) override;
    bool sendToAll(std::string_view message) override;

    void disconnect(ClientId clientId) override;
};

}

#endif
