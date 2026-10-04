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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSMESSAGE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSMESSAGE_HPP

#include "../cbusopcode.hpp"
#include "../../../../utils/byte.hpp"

namespace CBUS {

struct Message
{
  OpCode opCode;

  constexpr uint8_t size() const
  {
    return sizeof(OpCode) + dataSize(opCode);
  }

protected:
  Message(OpCode opc)
    : opCode{opc}
  {
  }
};

struct NodeMessage : Message
{
  uint8_t nodeNumberHigh;
  uint8_t nodeNumberLow;

  uint16_t nodeNumber() const
  {
    return to16(nodeNumberLow, nodeNumberHigh);
  }

protected:
  NodeMessage(OpCode opc, uint16_t nodeNumber_)
    : Message(opc)
    , nodeNumberHigh{high8(nodeNumber_)}
    , nodeNumberLow{low8(nodeNumber_)}
  {
  }
};

}

#endif
