/**
 * server/src/hardware/protocol/loconet/message/immpacket.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_IMMPACKET_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_IMMPACKET_HPP

#include "message.hpp"
#include <tcb/span.hpp>
#include "../checksum.hpp"
#include "../signature.hpp"

namespace LocoNet {

struct ImmPacket : Message
{
  static constexpr std::array<SignatureByte, 10> signature =
  {{
    {OPC_IMM_PACKET},
    {11, 0xFF},
    {0x7F, 0xFF},
    {0x00, 0x84},
    {0x20, 0xE0},
    {},
    {},
    {},
    {},
    {}
  }};

  static constexpr uint8_t dccPacketSizeMax = 5;
  static constexpr uint8_t repeatMax = 7;

  uint8_t len;
  uint8_t header;
  uint8_t reps;
  uint8_t dhi;
  uint8_t im[5];
  uint8_t checksum;

  ImmPacket(tcb::span<const uint8_t> dccPacket, uint8_t repeat)
    : Message(OPC_IMM_PACKET)
    , len{11}
    , header{0x7F}
    , reps{0}
    , dhi{0x20}
    , im{0, 0, 0, 0, 0}
  {
    assert(dccPacket.size() <= dccPacketSizeMax);
    assert(repeat <= repeatMax);
    const uint8_t dccPacketSize = std::min<size_t>(dccPacket.size(), dccPacketSizeMax);
    reps = (repeat & 0x07) | (dccPacketSize << 4);
    for(uint8_t i = 0; i < dccPacketSize; i++)
    {
      im[i] = dccPacket[i] & 0x7F;
      if(dccPacket[i] & 0x80)
        dhi |= 1 << i;
    }
    checksum = calcChecksum(*this);
  }
};
static_assert(sizeof(ImmPacket) == 11);

}

#endif
