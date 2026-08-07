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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_IOHANDLER_DINAMOSERIALIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_IOHANDLER_DINAMOSERIALIOHANDLER_HPP

#include "dinamoiohandler.hpp"
#include <boost/asio/serial_port.hpp>

namespace Dinamo {

class SerialIOHandler final : public IOHandler
{
public:
  SerialIOHandler(Kernel& kernel, const std::string& device);
  ~SerialIOHandler() final;

  void stop() final;

  [[nodiscard]] std::error_code send(std::span<const uint8_t> message, bool hold, bool fault) final;

private:
  boost::asio::serial_port m_serialPort;

  std::array<uint8_t, 64> m_rxBuffer;
  size_t m_rxBufferOffset = 0;

  std::array<uint8_t, 1024> m_txBuffer;
  size_t m_txBufferOffset = 0;
  bool m_txIdle = true;
  bool m_txToggle = false;
  uint8_t m_txRetries = 0;

  void read();
  void write();

  void startResponseTimeoutTimer();
};

}

#endif

