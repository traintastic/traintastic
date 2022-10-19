/**
 * server/src/hardware/protocol/loconet/messages/message.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGES_MESSAGE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGES_MESSAGE_HPP

#include "../opcode.hpp"

namespace LocoNet {

/**
 * \addtogroup loconet
 * \{
 *   \defgroup loconet_messages Messages
 *   \{
 *   \}
 * \}
 */

struct Message
{
  OpCode opCode;

  Message()
  {
  }

  Message(OpCode _opCode) :
    opCode{_opCode}
  {
  }

  uint8_t size() const
  {
    switch(opCode & 0xE0)
    {
      case 0x80: // 1 0 0 F D C B A
        return 2;

      case 0xA0: // 1 0 1 F D C B A
        return 4;

      case 0xC0: // 1 1 0 F D C B A
        return 6;

      case 0xE0: // 1 1 1 F D C B A => length in next byte
        return reinterpret_cast<const uint8_t*>(this)[1];

      default:
        return 0; // not an op opcode, bit 7 not 1
    }
  }
};

}

#endif
