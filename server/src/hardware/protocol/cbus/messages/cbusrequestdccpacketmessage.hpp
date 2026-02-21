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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSREQUESTDCCPACKETMESSAGE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSREQUESTDCCPACKETMESSAGE_HPP

#include "cbusmessage.hpp"
#include <cassert>
#include <cstring>
#include <span>

namespace CBUS {

template<size_t N>
requires(N >= 3 && N <= 6)
struct RequestDCCPacket : Message
{
  uint8_t repeat;
  uint8_t data[N];

  RequestDCCPacket(std::span<const uint8_t> bytes, uint8_t repeat_)
    : Message(RDCCn())
    , repeat{repeat_}
  {
    assert(N == bytes.size());
    std::memcpy(data, bytes.data(), N);
  }

private:
  static constexpr OpCode RDCCn() noexcept
  {
    switch(N)
    {
      case 3: return OpCode::RDCC3;
      case 4: return OpCode::RDCC4;
      case 5: return OpCode::RDCC5;
      case 6: return OpCode::RDCC6;
    }
    return static_cast<OpCode>(0); // unreachable, should never happen
  }
};
static_assert(sizeof(RequestDCCPacket<3>) == 5);
static_assert(sizeof(RequestDCCPacket<4>) == 6);
static_assert(sizeof(RequestDCCPacket<5>) == 7);
static_assert(sizeof(RequestDCCPacket<6>) == 8);

}

#endif
