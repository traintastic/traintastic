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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSACCESSORYMESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSACCESSORYMESSAGES_HPP

#include "cbusmessage.hpp"
#include "../../../../utils/byte.hpp"

namespace CBUS {

template<OpCode opc>
struct AccessoryMessage : Message
{
  uint8_t nodeNumberHigh;
  uint8_t nodeNumberLow;
  uint8_t eventNumberHigh;
  uint8_t eventNumberLow;

  AccessoryMessage(uint16_t nodeNumber_, uint16_t eventNumber_)
    : Message(opc)
    , nodeNumberHigh{high8(nodeNumber_)}
    , nodeNumberLow{low8(nodeNumber_)}
    , eventNumberHigh{high8(eventNumber_)}
    , eventNumberLow{low8(eventNumber_)}
  {
  }

  uint16_t nodeNumber() const
  {
    return to16(nodeNumberLow, nodeNumberHigh);
  }

  uint16_t eventNumber() const
  {
    return to16(eventNumberLow, eventNumberHigh);
  }
};

using AccessoryOn = AccessoryMessage<OpCode::ACON>;
using AccessoryOff = AccessoryMessage<OpCode::ACOF>;
using AccessoryRequestEvent = AccessoryMessage<OpCode::AREQ>;
using AccessoryResponseEventOn = AccessoryMessage<OpCode::ARON>;
using AccessoryResponseEventOff = AccessoryMessage<OpCode::AROF>;

static_assert(sizeof(AccessoryOn) == 5);
static_assert(sizeof(AccessoryOff) == 5);
static_assert(sizeof(AccessoryRequestEvent) == 5);
static_assert(sizeof(AccessoryResponseEventOn) == 5);
static_assert(sizeof(AccessoryResponseEventOff) == 5);

}

#endif
