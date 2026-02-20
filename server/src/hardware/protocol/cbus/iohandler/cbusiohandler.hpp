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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_IOHANDLER_CBUSIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_IOHANDLER_CBUSIOHANDLER_HPP

#include <cstdint>
#include <system_error>

namespace CBUS {

class Kernel;
struct Message;

class IOHandler
{
public:
  IOHandler(const IOHandler&) = delete;
  IOHandler& operator =(const IOHandler&) = delete;

  virtual ~IOHandler() = default;

  virtual void start() = 0;
  virtual void stop() = 0;

  [[nodiscard]] virtual std::error_code send(const Message& message) = 0;

protected:
  Kernel& m_kernel;
  const uint8_t m_canId;

  IOHandler(Kernel& kernel, uint8_t canId);
};

template<class T>
constexpr bool isSimulation()
{
  return false;
}

}

#endif
