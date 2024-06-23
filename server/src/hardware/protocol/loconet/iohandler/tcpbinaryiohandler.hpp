/**
 * server/src/hardware/protocol/loconet/iohandler/tcpbinaryiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023-2024 Reinder Feenstra
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


#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_IOHANDLER_TCPBINARYIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_IOHANDLER_TCPBINARYIOHANDLER_HPP

#include "tcpiohandler.hpp"

namespace LocoNet {

class TCPBinaryIOHandler final : public TCPIOHandler
{
  private:
    std::array<std::byte, 1500> m_readBuffer;
    size_t m_readBufferOffset;
    std::array<std::byte, 1500> m_writeBuffer;
    size_t m_writeBufferOffset;

  protected:
    void read() final;
    void write() final;

  public:
    TCPBinaryIOHandler(Kernel& kernel, std::string hostname, uint16_t port);

    bool send(const Message& message) final;
};

}

#endif

