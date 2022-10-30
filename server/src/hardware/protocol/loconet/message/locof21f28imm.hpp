/**
 * server/src/hardware/protocol/loconet/message/locof21f28imm.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_LOCOF21F28IMM_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_LOCOF21F28IMM_HPP

#include "message.hpp"
#include "../signature.hpp"

namespace LocoNet {

uint8_t calcChecksum(const Message&);

struct LocoF21F28IMMShortAddress : Message
{
  static constexpr std::array<SignatureByte, 10> signature =
  {{
    {OPC_IMM_PACKET},
    {11, 0xFF},
    {0x7F, 0xFF},
    {0x34, 0xFF},
    {0x02, 0xDB},
    {},
    {0x5F, 0xFF},
    {},
    {0x00, 0xFF},
    {0x00, 0xFF}
  }};

  static constexpr uint8_t F21 = 0x01;
  static constexpr uint8_t F22 = 0x02;
  static constexpr uint8_t F23 = 0x04;
  static constexpr uint8_t F24 = 0x08;
  static constexpr uint8_t F25 = 0x10;
  static constexpr uint8_t F26 = 0x20;
  static constexpr uint8_t F27 = 0x40;
  static constexpr uint8_t F28 = 0x04;

  uint8_t len = 11;
  uint8_t data1 = 0x7F;
  uint8_t data2 = 0x34;
  uint8_t data3 = 0x22;
  uint8_t data4 = 0;
  uint8_t data5 = 0x5F;
  uint8_t data6 = 0;
  uint8_t data7 = 0x00;
  uint8_t data8 = 0x00;
  uint8_t checksum;

  LocoF21F28IMMShortAddress(uint8_t shortAddress, bool f21on, bool f22on, bool f23on, bool f24on, bool f25on, bool f26on, bool f27on, bool f28on)
    : Message(OPC_IMM_PACKET)
  {
    setAddress(shortAddress);
    setF21(f21on);
    setF22(f22on);
    setF23(f23on);
    setF24(f24on);
    setF25(f25on);
    setF26(f26on);
    setF27(f27on);
    setF28(f28on);
    checksum = calcChecksum(*this);
  }

  uint8_t address() const
  {
    return data4;
  }

  void setAddress(uint8_t value)
  {
    data4 = value & 0x7F;
  }

  bool f(uint8_t n) const
  {
    assert(n >= 21 && n <= 28);
    return (n == 28) ? f28() : (data6 & (1 << (n - 21)));
  }

  bool f21() const
  {
    return data6 & F21;
  }

  void setF21(bool on)
  {
    if(on)
      data6 |= F21;
    else
      data6 &= ~F21;
  }

  bool f22() const
  {
    return data6 & F22;
  }

  void setF22(bool on)
  {
    if(on)
      data6 |= F22;
    else
      data6 &= ~F22;
  }

  bool f23() const
  {
    return data6 & F23;
  }

  void setF23(bool on)
  {
    if(on)
      data6 |= F23;
    else
      data6 &= ~F23;
  }

  bool f24() const
  {
    return data6 & F24;
  }

  void setF24(bool on)
  {
    if(on)
      data6 |= F24;
    else
      data6 &= ~F24;
  }

  bool f25() const
  {
    return data6 & F25;
  }

  void setF25(bool on)
  {
    if(on)
      data6 |= F25;
    else
      data6 &= ~F25;
  }

  bool f26() const
  {
    return data6 & F26;
  }

  void setF26(bool on)
  {
    if(on)
      data6 |= F26;
    else
      data6 &= ~F26;
  }

  bool f27() const
  {
    return data6 & F27;
  }

  void setF27(bool on)
  {
    if(on)
      data6 |= F27;
    else
      data6 &= ~F27;
  }

  bool f28() const
  {
    return data3 & F28;
  }

  void setF28(bool on)
  {
    if(on)
      data3 |= F28;
    else
      data3 &= ~F28;
  }
};
static_assert(sizeof(LocoF21F28IMMShortAddress) == 11);

struct LocoF21F28IMMLongAddress : Message
{
  static constexpr std::array<SignatureByte, 10> signature =
  {{
    {OPC_IMM_PACKET},
    {11, 0xFF},
    {0x7F, 0xFF},
    {0x44, 0xFF},
    {0x05, 0xD5},
    {0x40, 0xC0},
    {},
    {0x5F, 0xFF},
    {},
    {0x00, 0xFF}
  }};

  static constexpr uint8_t F21 = 0x01;
  static constexpr uint8_t F22 = 0x02;
  static constexpr uint8_t F23 = 0x04;
  static constexpr uint8_t F24 = 0x08;
  static constexpr uint8_t F25 = 0x10;
  static constexpr uint8_t F26 = 0x20;
  static constexpr uint8_t F27 = 0x40;
  static constexpr uint8_t F28 = 0x08;

  uint8_t len = 11;
  uint8_t data1 = 0x7F;
  uint8_t data2 = 0x44;
  uint8_t data3 = 0x25;
  uint8_t data4 = 0x40;
  uint8_t data5 = 0;
  uint8_t data6 = 0x5F;
  uint8_t data7 = 0;
  uint8_t data8 = 0x00;
  uint8_t checksum;

  LocoF21F28IMMLongAddress(uint16_t longAddress, bool f21on, bool f22on, bool f23on, bool f24on, bool f25on, bool f26on, bool f27on, bool f28on)
    : Message(OPC_IMM_PACKET)
  {
    setAddress(longAddress);
    setF21(f21on);
    setF22(f22on);
    setF23(f23on);
    setF24(f24on);
    setF25(f25on);
    setF26(f26on);
    setF27(f27on);
    setF28(f28on);
    checksum = calcChecksum(*this);
  }

  uint16_t address() const
  {
    return (static_cast<uint16_t>(data4 & 0x3F) << 8) | ((data3 & 0x02) << 6) | (data5 & 0x7F);
  }

  void setAddress(uint16_t value)
  {
    data3 = (data3 & 0x7D) | ((value & 0x80) >> 6);
    data4 = 0x40 | ((value >> 8) & 0x3F);
    data5 = value & 0x7F;
  }

  bool f(uint8_t n) const
  {
    assert(n >= 21 && n <= 28);
    return (n == 28) ? f28() : (data7 & (1 << (n - 21)));
  }

  bool f21() const
  {
    return data7 & F21;
  }

  void setF21(bool on)
  {
    if(on)
      data7 |= F21;
    else
      data7 &= ~F21;
  }

  bool f22() const
  {
    return data7 & F22;
  }

  void setF22(bool on)
  {
    if(on)
      data7 |= F22;
    else
      data7 &= ~F22;
  }

  bool f23() const
  {
    return data7 & F23;
  }

  void setF23(bool on)
  {
    if(on)
      data7 |= F23;
    else
      data7 &= ~F23;
  }

  bool f24() const
  {
    return data7 & F24;
  }

  void setF24(bool on)
  {
    if(on)
      data7 |= F24;
    else
      data7 &= ~F24;
  }

  bool f25() const
  {
    return data7 & F25;
  }

  void setF25(bool on)
  {
    if(on)
      data7 |= F25;
    else
      data7 &= ~F25;
  }

  bool f26() const
  {
    return data7 & F26;
  }

  void setF26(bool on)
  {
    if(on)
      data7 |= F26;
    else
      data7 &= ~F26;
  }

  bool f27() const
  {
    return data7 & F27;
  }

  void setF27(bool on)
  {
    if(on)
      data7 |= F27;
    else
      data7 &= ~F27;
  }

  bool f28() const
  {
    return data3 & F28;
  }

  void setF28(bool on)
  {
    if(on)
      data3 |= F28;
    else
      data3 &= ~F28;
  }
};
static_assert(sizeof(LocoF21F28IMMLongAddress) == 11);

}

#endif
