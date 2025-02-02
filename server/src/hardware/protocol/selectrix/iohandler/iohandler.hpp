/**
 * server/src/hardware/protocol/selectrix/iohandler/iohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023,2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_IOHANDLER_IOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_IOHANDLER_IOHANDLER_HPP

#include <cstdint>
#include "../bus.hpp"

namespace Selectrix {

class Kernel;

class IOHandler
{
  protected:
    Kernel& m_kernel;

    IOHandler(Kernel& kernel)
      : m_kernel{kernel}
    {
    }

  public:
    IOHandler(const IOHandler&) = delete;
    IOHandler& operator =(const IOHandler&) = delete;

    virtual ~IOHandler() = default;

    virtual bool requiresPolling() const = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual bool read(Bus bus, uint8_t address) = 0;
    virtual bool write(Bus bus, uint8_t address, uint8_t value) = 0;
};

template<class T>
constexpr bool isSimulation()
{
  return false;
}

}

#endif
