/**
 * server/src/hardware/protocol/loconet/message/uhlenbrock/lncv.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_UHLENBROCK_LNCV_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_UHLENBROCK_LNCV_HPP

#include "datamessage.hpp"
#include "../../checksum.hpp"

namespace LocoNet::Uhlenbrock {

static constexpr uint16_t lncvBroadcastAddress = 65535;

struct LNCVStart : ImmPacketDataMessage
{
  static constexpr std::array<uint8_t, dataLen> magicData = {0x01, 0x05, 0x00, 0x21, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  static constexpr std::array<uint8_t, dataLen> magicMask = {0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x80, 0x80, 0xFF, 0xFF, 0x80, 0x80, 0xFF};

  inline static bool check(const Message& message)
  {
    return
      ImmPacketDataMessage::check(message) &&
      checkMagicData<LNCVStart>(static_cast<const ImmPacketDataMessage&>(message));
  }

  LNCVStart()
  {
    setMagicData(*this);
  }

  LNCVStart(uint16_t moduleId, uint16_t address)
    : LNCVStart()
  {
    setModuleId(moduleId);
    setAddress(address);

    checksum = calcChecksum(*this);
  }

  uint16_t moduleId() const
  {
    return to16(getData<5>(), getData<6>());
  }

  void setModuleId(uint16_t value)
  {
    setData<5>(low8(value));
    setData<6>(high8(value));
  }

  uint16_t address() const
  {
    return to16(getData<9>(), getData<10>());
  }

  void setAddress(uint16_t value)
  {
    setData<9>(low8(value));
    setData<10>(high8(value));
  }
};
static_assert(sizeof(LNCVStart) == 15);

struct LNCVRead : ImmPacketDataMessage
{
  static constexpr std::array<uint8_t, dataLen> magicData = {0x01, 0x05, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  static constexpr std::array<uint8_t, dataLen> magicMask = {0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF};

  inline static bool check(const Message& message)
  {
    return
      ImmPacketDataMessage::check(message) &&
      checkMagicData<LNCVRead>(static_cast<const ImmPacketDataMessage&>(message));
  }

  LNCVRead()
  {
    setMagicData(*this);
  }

  LNCVRead(uint16_t moduleId, uint16_t address, uint16_t lncv)
    : LNCVRead()
  {
    setModuleId(moduleId);
    setAddress(address);
    setLNCV(lncv);

    checksum = calcChecksum(*this);
  }

  uint16_t moduleId() const
  {
    return to16(getData<5>(), getData<6>());
  }

  void setModuleId(uint16_t value)
  {
    setData<5>(low8(value));
    setData<6>(high8(value));
  }

  uint16_t lncv() const
  {
    return to16(getData<7>(), getData<8>());
  }

  void setLNCV(uint16_t value)
  {
    setData<7>(low8(value));
    setData<8>(high8(value));
  }

  uint16_t address() const
  {
    return to16(getData<9>(), getData<10>());
  }

  void setAddress(uint16_t value)
  {
    setData<9>(low8(value));
    setData<10>(high8(value));
  }
};
static_assert(sizeof(LNCVRead) == 15);

struct LNCVReadResponse : PeerXferDataMessage
{
  static constexpr std::array<uint8_t, dataLen> magicData = {0x05, 0x49, 0x4B, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  static constexpr std::array<uint8_t, dataLen> magicMask = {0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF};

  inline static bool check(const Message& message)
  {
    return
      PeerXferDataMessage::check(message) &&
      checkMagicData<LNCVReadResponse>(static_cast<const PeerXferDataMessage&>(message));
  }

  LNCVReadResponse()
  {
    setMagicData(*this);
  }

  LNCVReadResponse(uint16_t moduleId, uint16_t lncv, uint16_t value)
    : LNCVReadResponse()
  {
    setModuleId(moduleId);
    setLNCV(lncv);
    setValue(value);

    checksum = calcChecksum(*this);
  }

  uint16_t moduleId() const
  {
    return to16(getData<5>(), getData<6>());
  }

  void setModuleId(uint16_t value)
  {
    setData<5>(low8(value));
    setData<6>(high8(value));
  }

  uint16_t lncv() const
  {
    return to16(getData<7>(), getData<8>());
  }

  void setLNCV(uint16_t value)
  {
    setData<7>(low8(value));
    setData<8>(high8(value));
  }

  uint16_t value() const
  {
    return to16(getData<9>(), getData<10>());
  }

  void setValue(uint16_t value)
  {
    setData<9>(low8(value));
    setData<10>(high8(value));
  }
};
static_assert(sizeof(LNCVReadResponse) == 15);

struct LNCVWrite : ImmPacketDataMessage
{
  static constexpr std::array<uint8_t, dataLen> magicData = {0x01, 0x05, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  static constexpr std::array<uint8_t, dataLen> magicMask = {0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF};

  inline static bool check(const Message& message)
  {
    return
      ImmPacketDataMessage::check(message) &&
      checkMagicData<LNCVWrite>(static_cast<const ImmPacketDataMessage&>(message));
  }

  LNCVWrite()
  {
    setMagicData(*this);
  }

  LNCVWrite(uint16_t moduleId, uint16_t lncv, uint16_t value)
    : LNCVWrite()
  {
    setModuleId(moduleId);
    setLNCV(lncv);
    setValue(value);

    checksum = calcChecksum(*this);
  }

  uint16_t moduleId() const
  {
    return to16(getData<5>(), getData<6>());
  }

  void setModuleId(uint16_t value)
  {
    setData<5>(low8(value));
    setData<6>(high8(value));
  }

  uint16_t lncv() const
  {
    return to16(getData<7>(), getData<8>());
  }

  void setLNCV(uint16_t value)
  {
    setData<7>(low8(value));
    setData<8>(high8(value));
  }

  uint16_t value() const
  {
    return to16(getData<9>(), getData<10>());
  }

  void setValue(uint16_t value)
  {
    setData<9>(low8(value));
    setData<10>(high8(value));
  }
};
static_assert(sizeof(LNCVWrite) == 15);

struct LNCVStop : ImmPacketDataMessage
{
  static constexpr std::array<uint8_t, dataLen> magicData = {0x01, 0x05, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40};
  static constexpr std::array<uint8_t, dataLen> magicMask = {0xFF, 0xFF, 0xFF, 0xFF, 0xCC, 0x80, 0x80, 0xFF, 0xFF, 0x80, 0x80, 0xFF};

  inline static bool check(const Message& message)
  {
    return
      ImmPacketDataMessage::check(message) &&
      checkMagicData<LNCVStop>(static_cast<const ImmPacketDataMessage&>(message));
  }

  LNCVStop()
  {
    setMagicData(*this);
  }

  LNCVStop(uint16_t moduleId, uint16_t address)
    : LNCVStop()
  {
    setModuleId(moduleId);
    setAddress(address);

    checksum = calcChecksum(*this);
  }

  uint16_t moduleId() const
  {
    return to16(getData<5>(), getData<6>());
  }

  void setModuleId(uint16_t value)
  {
    setData<5>(low8(value));
    setData<6>(high8(value));
  }

  uint16_t address() const
  {
    return to16(getData<9>(), getData<10>());
  }

  void setAddress(uint16_t value)
  {
    setData<9>(low8(value));
    setData<10>(high8(value));
  }
};
static_assert(sizeof(LNCVStop) == 15);

}

#endif
