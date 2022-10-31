/**
 * server/src/hardware/protocol/loconet/message/uhlenbrock/specialoption.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_UHLENBROCK_SPECIALOPTION_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_UHLENBROCK_SPECIALOPTION_HPP

#include "datamessage.hpp"
#include "../../checksum.hpp"

namespace LocoNet::Uhlenbrock {

using SpecialOption = uint16_t;

struct ReadSpecialOption : ImmPacketDataMessage
{
  static constexpr std::array<uint8_t, dataLen> magicData = {0x01, 0x49, 0x42, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  static constexpr std::array<uint8_t, dataLen> magicMask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x80, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  inline static bool check(const Message& message)
  {
    return
      ImmPacketDataMessage::check(message) &&
      checkMagicData<ReadSpecialOption>(static_cast<const ImmPacketDataMessage&>(message));
  }

  ReadSpecialOption(SpecialOption specialOption)
  {
    setMagicData(*this);

    setData<5>(specialOption & 0xFF);
    setData<6>(specialOption >> 8);

    checksum = calcChecksum(*this);
  }

  SpecialOption specialOption() const
  {
    return (static_cast<SpecialOption>(getData<6>()) << 8) | getData<5>();
  }
};
static_assert(sizeof(ReadSpecialOption) == 15);

struct ReadSpecialOptionReply : PeerXferDataMessage
{
  static constexpr std::array<uint8_t, dataLen> magicData = {0x00, 0x49, 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  static constexpr std::array<uint8_t, dataLen> magicMask = {0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x80, 0x80, 0x80, 0xFF, 0xFF, 0xFF, 0xFF};

  inline static bool check(const Message& message)
  {
    return
      PeerXferDataMessage::check(message) &&
      checkMagicData<ReadSpecialOptionReply>(static_cast<const PeerXferDataMessage&>(message));
  }

  ReadSpecialOptionReply(SpecialOption specialOption, uint8_t value)
  {
    setMagicData(*this);

    setData<5>(low8(specialOption));
    setData<6>(high8(specialOption));
    setData<7>(value);

    checksum = calcChecksum(*this);
  }

  SpecialOption specialOption() const
  {
    return to16(getData<5>(), getData<6>());
  }

  uint8_t value() const
  {
    return getData<7>();
  }
};
static_assert(sizeof(ReadSpecialOptionReply) == 15);

}

#endif
