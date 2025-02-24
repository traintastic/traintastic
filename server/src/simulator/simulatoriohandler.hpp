/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_SIMULATOR_SIMULATORIOHANDLER_HPP
#define TRAINTASTIC_SERVER_SIMULATOR_SIMULATORIOHANDLER_HPP

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace SimulatorProtocol
{
  struct Message;
}

enum class DecoderProtocol : uint8_t;
enum class Direction : uint8_t;

class SimulatorIOHandler
{
  private:
    const std::string m_hostname;
    const uint16_t m_port;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::endpoint m_endpoint;
    bool m_connected = false;
    std::array<std::byte, 1024> m_readBuffer;
    size_t m_readBufferOffset = 0;
    std::array<std::byte, 1024> m_writeBuffer;
    size_t m_writeBufferOffset = 0;

    bool send(const SimulatorProtocol::Message& message);
    void receive(const SimulatorProtocol::Message& message);

    void read();
    void write();

  public:
    SimulatorIOHandler(boost::asio::io_context& ioContext, std::string hostname, uint16_t port);

    void start();
    void stop();

    void sendLocomotiveSpeedDirection(DecoderProtocol protocol, uint16_t address, uint8_t speed, Direction direction, bool emergencyStop);

    std::function<void(DecoderProtocol, uint16_t, uint8_t, Direction, bool)> onLocomotiveSpeedDirection;
    std::function<void(uint16_t, uint16_t, bool)> onSensorChanged;
};

#endif
