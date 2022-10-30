/**
 * server/src/hardware/protocol/loconet/signature.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_SIGNATURE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_SIGNATURE_HPP

#include "message/message.hpp"
#include <cassert>

namespace LocoNet {

struct SignatureByte
{
  uint8_t value;
  uint8_t mask;

  constexpr SignatureByte(uint8_t value_, uint8_t mask_)
    : value{value_}
    , mask{mask_}
  {
  }

  constexpr SignatureByte(OpCode opCode)
    : SignatureByte(static_cast<uint8_t>(opCode), 0xFF)
  {
  }

  constexpr SignatureByte()
    : SignatureByte(0x00, 0x80)
  {
  }
};

template<class T>
bool isSignatureMatch(const Message& message)
{
  static_assert(T::signature.size() == sizeof(T) - 1); // all except checksum
  if(message.size() != sizeof(T))
    return false;
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&message);
  for(const auto& byte : T::signature)
  {
    if((*p & byte.mask) != byte.value)
      return false;
    p++;
  }
  return true;
}

}

#endif
