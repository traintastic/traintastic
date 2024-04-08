/**
 * server/src/hardware/protocol/dccex/iohandler/hardwareiohandler.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCCEX_IOHANDLER_HARDWAREIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCCEX_IOHANDLER_HARDWAREIOHANDLER_HPP

#include <array>
#include "iohandler.hpp"

namespace DCCEX {

class Kernel;

class HardwareIOHandler : public IOHandler
{
  protected:
    std::array<char, 1024> m_readBuffer;
    size_t m_readBufferOffset;
    std::array<char, 1024> m_writeBuffer;
    size_t m_writeBufferOffset;

    HardwareIOHandler(Kernel& kernel);

    void processRead(size_t bytesTransferred);
    virtual void write() = 0;

  public:
    bool send(std::string_view message) final;
};

}

#endif
