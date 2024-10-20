/**
 * server/src/hardware/protocol/dinamo/iohandler/serialiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_IOHANDLER_SERIALIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_IOHANDLER_SERIALIOHANDLER_HPP

#include "iohandler.hpp"
#include <array>
#include <cstddef>
#include <boost/asio/serial_port.hpp>

namespace Dinamo {

class SerialIOHandler final : public IOHandler
{
private:
  boost::asio::serial_port m_serialPort;
  std::array<std::byte, 1024> m_readBuffer;
  size_t m_readBufferOffset = 0;
  std::array<std::byte, 64> m_writeBuffer;
  size_t m_writeBufferOffset = 0;

  void read();
  void write();

public:
  SerialIOHandler(Kernel& kernel, const std::string& device);
  ~SerialIOHandler() final;

  void start() final;
  void stop() final;

  bool send(const Message& message) final;
};

}

#endif
