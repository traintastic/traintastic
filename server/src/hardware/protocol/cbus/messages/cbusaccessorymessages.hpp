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

namespace CBUS {

template<OpCode opc>
struct AccessoryMessage : NodeMessage
{
  uint8_t eventNumberHigh;
  uint8_t eventNumberLow;

  AccessoryMessage(uint16_t nodeNumber_, uint16_t eventNumber_)
    : NodeMessage(opc, nodeNumber_)
    , eventNumberHigh{high8(eventNumber_)}
    , eventNumberLow{low8(eventNumber_)}
  {
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

template<OpCode opc>
struct Accessory2Message : AccessoryMessage<opc>
{
  uint8_t data1;
  uint8_t data2;

  Accessory2Message(uint16_t nodeNumber_, uint16_t eventNumber_, uint8_t data1_, uint8_t data2_)
    : AccessoryMessage<opc>(nodeNumber_, eventNumber_)
    , data1{data1_}
    , data2{data2_}
  {
  }

  Accessory2Message(uint16_t nodeNumber_, uint16_t eventNumber_, uint16_t data_)
    : Accessory2Message(opc, nodeNumber_, eventNumber_, high8(data_), low8(data_))
  {
  }

  uint16_t data() const
  {
    return to16(data2, data1);
  }
};

using Accessory2On = Accessory2Message<OpCode::ACON2>;
using Accessory2Off = Accessory2Message<OpCode::ACOF2>;
using Accessory2ResponseEventOn = Accessory2Message<OpCode::ARON2>;
using Accessory2ResponseEventOff = Accessory2Message<OpCode::AROF2>;

static_assert(sizeof(Accessory2On) == 7);
static_assert(sizeof(Accessory2Off) == 7);
static_assert(sizeof(Accessory2ResponseEventOn) == 7);
static_assert(sizeof(Accessory2ResponseEventOff) == 7);

}

#endif
