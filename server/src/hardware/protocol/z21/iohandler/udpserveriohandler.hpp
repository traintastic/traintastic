/**
 * server/src/hardware/protocol/z21/iohandler/udpserveriohandler.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_IOHANDLER_UDPSERVERIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_IOHANDLER_UDPSERVERIOHANDLER_HPP

#include "udpiohandler.hpp"
#include <unordered_map>

namespace Z21 {

class ServerKernel;

class UDPServerIOHandler final : public UDPIOHandler
{
  private:
    ClientId m_lastClientId = 0;
    std::unordered_map<ClientId, boost::asio::ip::udp::endpoint> m_clients;

  protected:
    void receive(const Message& message, const boost::asio::ip::udp::endpoint& remoteEndpoint) final;

  public:
    UDPServerIOHandler(ServerKernel& kernel);

    bool sendTo(const Message& message, ClientId id) final;

    void purgeClient(ClientId id) final;
};

}

#endif
