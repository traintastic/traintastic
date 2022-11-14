/**
 * server/src/hardware/protocol/loconet/message/slot.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_SLOT_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_SLOT_HPP

#include "message.hpp"

namespace LocoNet {

constexpr uint8_t SLOT_FAST_CLOCK = 123;

constexpr uint16_t idTraintastic = 0x3FF3;

struct SlotDataBase : Message
{
  uint8_t len;
  uint8_t slot;

  SlotDataBase(OpCode opcode_, uint8_t slot_ = 0)
    : Message(opcode_)
    , len{14}
    , slot{slot_}
  {
  }
};

}

#endif
