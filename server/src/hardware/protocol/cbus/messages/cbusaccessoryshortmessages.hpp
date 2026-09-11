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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSACCESSORYSHORTMESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSACCESSORYSHORTMESSAGES_HPP

#include "cbusmessage.hpp"

namespace CBUS {

template<OpCode opc>
struct AccessoryShortMessage : NodeMessage
{
  uint8_t deviceNumberHigh;
  uint8_t deviceNumberLow;

  AccessoryShortMessage(uint16_t nodeNumber_, uint16_t deviceNumber_)
    : NodeMessage(opc, nodeNumber_)
    , deviceNumberHigh{high8(deviceNumber_)}
    , deviceNumberLow{low8(deviceNumber_)}
  {
  }

  uint16_t deviceNumber() const
  {
    return to16(deviceNumberLow, deviceNumberHigh);
  }
};

using AccessoryShortOn = AccessoryShortMessage<OpCode::ASON>;
using AccessoryShortOff = AccessoryShortMessage<OpCode::ASOF>;
using AccessoryShortRequestEvent = AccessoryShortMessage<OpCode::ASRQ>;
using AccessoryShortResponseEventOn = AccessoryShortMessage<OpCode::ARSON>;
using AccessoryShortResponseEventOff = AccessoryShortMessage<OpCode::ARSOF>;

static_assert(sizeof(AccessoryShortOn) == 5);
static_assert(sizeof(AccessoryShortOff) == 5);
static_assert(sizeof(AccessoryShortRequestEvent) == 5);
static_assert(sizeof(AccessoryShortResponseEventOn) == 5);
static_assert(sizeof(AccessoryShortResponseEventOff) == 5);

}

#endif
