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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_SIMULATOR_SIMULATORCONNECTION_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_SIMULATOR_SIMULATORCONNECTION_HPP

#include <array>
#include <memory>
#include <boost/asio/ip/tcp.hpp>

namespace SimulatorProtocol {
  struct Message;
}

class Simulator;

class SimulatorConnection : public std::enable_shared_from_this<SimulatorConnection>
{
private:
  std::shared_ptr<Simulator> m_simulator;
  boost::asio::ip::tcp::socket m_socket;
  std::array<std::byte, 1024> m_readBuffer;
  size_t m_readBufferOffset = 0;
  std::array<std::byte, 1024> m_writeBuffer;
  size_t m_writeBufferOffset = 0;
  size_t m_connectionId = 0;
  bool m_handShakeResponseReceived = true;

  void read();
  void write();
  void close();

public:
  SimulatorConnection(std::shared_ptr<Simulator> simulator, boost::asio::ip::tcp::socket&& socket,
                      size_t connId);

  void start();
  void stop();

  bool send(const SimulatorProtocol::Message& message);

  bool handShakeResponseReceived() const { return m_handShakeResponseReceived; }

  void setHandShakeResponseReceived(bool newHandShakeResponseReceived)
  {
    m_handShakeResponseReceived = newHandShakeResponseReceived;
  }

  inline size_t connectionId() const { return m_connectionId; }
};

#endif
