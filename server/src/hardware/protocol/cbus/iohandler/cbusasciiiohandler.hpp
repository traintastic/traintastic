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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_IOHANDLER_CBUSASCIIIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_IOHANDLER_CBUSASCIIIOHANDLER_HPP

#include "cbusiohandler.hpp"
#include <array>
#include <string>
#include <queue>

namespace CBUS {

class ASCIIIOHandler : public IOHandler
{
public:
  [[nodiscard]] std::error_code send(const Message& message) override;

protected:
  std::array<char, 1024> m_readBuffer;
  size_t m_readBufferOffset;
  std::queue<std::string> m_writeQueue;

  ASCIIIOHandler(Kernel& kernel, uint8_t canId);

  void logDropIfNonZeroAndReset(size_t& drop);
  void processRead(std::size_t bytesTransferred);

  virtual void read() = 0;
  virtual void write() = 0;
};

}

#endif
