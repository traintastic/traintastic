/**
 * server/src/hardware/protocol/marklincan/iohandler/networkiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_IOHANDLER_NETWORKIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_IOHANDLER_NETWORKIOHANDLER_HPP

#include "iohandler.hpp"
#include <array>
#include "../messages.hpp"
#include "../../../../utils/packed.hpp"

namespace MarklinCAN {

class NetworkIOHandler : public IOHandler
{
  protected:
    PRAGMA_PACK_PUSH_1
    struct NetworkMessage
    {
      uint32_t idBE;
      uint8_t dlc;
      uint8_t data[8];
    } ATTRIBUTE_PACKED;
    PRAGMA_PACK_POP
    static_assert(sizeof(NetworkMessage) == 13);

    std::array<std::byte, 1500> m_writeBuffer;
    size_t m_writeBufferOffset;

    NetworkIOHandler(Kernel& kernel)
      : IOHandler(kernel)
      , m_writeBufferOffset{0}
    {
    }

    virtual void read() = 0;
    virtual void write() = 0;

    static Message toMessage(const NetworkMessage& networkMessage);
    static void toNetworkMessage(const Message& message, NetworkMessage& networkMessage);

  public:
    void start() override;

    bool send(const Message& message) final;
};

}

#endif
