/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_IOHANDLER_CBUSCANETHERIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_IOHANDLER_CBUSCANETHERIOHANDLER_HPP

#include "cbusasciiiohandler.hpp"
#include <boost/asio/ip/tcp.hpp>

namespace CBUS {

class CANEtherIOHandler final : public ASCIIIOHandler
{
public:
  CANEtherIOHandler(Kernel& kernel, std::string hostname, uint16_t port);

  void start() final;
  void stop() final;

protected:
  void read() final;
  void write() final;

private:
  static constexpr uint8_t canId = 0x7D; //!< CANEther fixed CAN_ID

  const std::string m_hostname;
  const uint16_t m_port;
  boost::asio::ip::tcp::socket m_socket;
  boost::asio::ip::tcp::endpoint m_endpoint;
  bool m_connected = false;
};

}

#endif
