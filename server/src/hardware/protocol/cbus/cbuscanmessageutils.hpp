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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSCANMESSAGEUTILS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSCANMESSAGEUTILS_HPP

#include "messages/cbusmessage.hpp"
#include <cstring>
#include "cbusgetminorpriority.hpp"
#include "cbuspriority.hpp"
#include "../can/canmessage.hpp"

namespace CBUS {

inline CAN::Message toCANMessage(const Message& message, uint8_t canId, MajorPriority majorPriority = MajorPriority::Lowest)
{
  CAN::Message canMessage;
  canMessage.id =
    (static_cast<uint32_t>(majorPriority) << 9) |
    (static_cast<uint32_t>(getMinorPriority(message.opCode)) << 7) |
    (canId & 0x7F),
  canMessage.rtr = false;
  canMessage.extended = false;
  canMessage.dlc = sizeof(OpCode) + dataSize(message.opCode);
  std::memcpy(canMessage.data, &message, canMessage.dlc);
  return canMessage;
}

inline const Message& asMessage(const CAN::Message& canMessage)
{
  return *reinterpret_cast<const Message*>(canMessage.data);
}

constexpr MajorPriority getMajorPriority(const CAN::Message& canMessage)
{
  return static_cast<MajorPriority>((canMessage.id >> 9) & 0b11);
}

constexpr MinorPriority getMinorPriority(const CAN::Message& canMessage)
{
  return static_cast<MinorPriority>((canMessage.id >> 7) & 0b11);
}

constexpr uint8_t getCanId(const CAN::Message& canMessage)
{
  return (canMessage.id & 0x7F);
}

}

#endif
