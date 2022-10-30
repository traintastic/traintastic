/**
 * server/src/hardware/protocol/loconet/message/locof13f20imm.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_LOCOF13F20IMM_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_LOCOF13F20IMM_HPP

#include "message.hpp"
#include "../signature.hpp"

namespace LocoNet {

uint8_t calcChecksum(const Message&);

struct LocoF13F20IMMShortAddress : Message
{
  static constexpr std::array<SignatureByte, 10> signature =
  {{
    {OPC_IMM_PACKET},
    {11, 0xFF},
    {0x7F, 0xFF},
    {0x34, 0xFF},
    {0x02, 0xDB},
    {},
    {0x5E, 0xFF},
    {},
    {0x00, 0xFF},
    {0x00, 0xFF}
  }};

  static constexpr uint8_t F13 = 0x01;
  static constexpr uint8_t F14 = 0x02;
  static constexpr uint8_t F15 = 0x04;
  static constexpr uint8_t F16 = 0x08;
  static constexpr uint8_t F17 = 0x10;
  static constexpr uint8_t F18 = 0x20;
  static constexpr uint8_t F19 = 0x40;
  static constexpr uint8_t F20 = 0x04;

  uint8_t len = 11;
  uint8_t data1 = 0x7F;
  uint8_t data2 = 0x34;
  uint8_t data3 = 0x22;
  uint8_t data4 = 0;
  uint8_t data5 = 0x5E;
  uint8_t data6 = 0;
  uint8_t data7 = 0x00;
  uint8_t data8 = 0x00;
  uint8_t checksum;

  LocoF13F20IMMShortAddress(uint8_t shortAddress, bool f13on, bool f14on, bool f15on, bool f16on, bool f17on, bool f18on, bool f19on, bool f20on)
    : Message(OPC_IMM_PACKET)
  {
    setAddress(shortAddress);
    setF13(f13on);
    setF14(f14on);
    setF15(f15on);
    setF16(f16on);
    setF17(f17on);
    setF18(f18on);
    setF19(f19on);
    setF20(f20on);
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
    assert(n >= 13 && n <= 20);
    return (n == 20) ? f20() : (data6 & (1 << (n - 13)));
  }

  bool f13() const
  {
    return data6 & F13;
  }

  void setF13(bool on)
  {
    if(on)
      data6 |= F13;
    else
      data6 &= ~F13;
  }

  bool f14() const
  {
    return data6 & F14;
  }

  void setF14(bool on)
  {
    if(on)
      data6 |= F14;
    else
      data6 &= ~F14;
  }

  bool f15() const
  {
    return data6 & F15;
  }

  void setF15(bool on)
  {
    if(on)
      data6 |= F15;
    else
      data6 &= ~F15;
  }

  bool f16() const
  {
    return data6 & F16;
  }

  void setF16(bool on)
  {
    if(on)
      data6 |= F16;
    else
      data6 &= ~F16;
  }

  bool f17() const
  {
    return data6 & F17;
  }

  void setF17(bool on)
  {
    if(on)
      data6 |= F17;
    else
      data6 &= ~F17;
  }

  bool f18() const
  {
    return data6 & F18;
  }

  void setF18(bool on)
  {
    if(on)
      data6 |= F18;
    else
      data6 &= ~F18;
  }

  bool f19() const
  {
    return data6 & F19;
  }

  void setF19(bool on)
  {
    if(on)
      data6 |= F19;
    else
      data6 &= ~F19;
  }

  bool f20() const
  {
    return data3 & F20;
  }

  void setF20(bool on)
  {
    if(on)
      data3 |= F20;
    else
      data3 &= ~F20;
  }
};
static_assert(sizeof(LocoF13F20IMMShortAddress) == 11);

struct LocoF13F20IMMLongAddress : Message
{
  static constexpr std::array<SignatureByte, 10> signature =
  {{
    {OPC_IMM_PACKET},
    {11, 0xFF},
    {0x7F, 0xFF},
    {0x44, 0xFF},
    {0x05, 0xD5},
    {0x40, 0x40},
    {},
    {0x5E, 0xFF},
    {},
    {0x00, 0xFF}
  }};

  static constexpr uint8_t F13 = 0x01;
  static constexpr uint8_t F14 = 0x02;
  static constexpr uint8_t F15 = 0x04;
  static constexpr uint8_t F16 = 0x08;
  static constexpr uint8_t F17 = 0x10;
  static constexpr uint8_t F18 = 0x20;
  static constexpr uint8_t F19 = 0x40;
  static constexpr uint8_t F20 = 0x08;

  uint8_t len = 11;
  uint8_t data1 = 0x7F;
  uint8_t data2 = 0x44;
  uint8_t data3 = 0x25;
  uint8_t data4 = 0;
  uint8_t data5 = 0x40;
  uint8_t data6 = 0x5E;
  uint8_t data7 = 0;
  uint8_t data8 = 0x00;
  uint8_t checksum;

  LocoF13F20IMMLongAddress(uint16_t longAddress, bool f13on, bool f14on, bool f15on, bool f16on, bool f17on, bool f18on, bool f19on, bool f20on)
    : Message(OPC_IMM_PACKET)
  {
    setAddress(longAddress);
    setF13(f13on);
    setF14(f14on);
    setF15(f15on);
    setF16(f16on);
    setF17(f17on);
    setF18(f18on);
    setF19(f19on);
    setF20(f20on);
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
    assert(n >= 13 && n <= 20);
    return (n == 20) ? f20() : (data7 & (1 << (n - 13)));
  }

  bool f13() const
  {
    return data7 & F13;
  }

  void setF13(bool on)
  {
    if(on)
      data7 |= F13;
    else
      data7 &= ~F13;
  }

  bool f14() const
  {
    return data7 & F14;
  }

  void setF14(bool on)
  {
    if(on)
      data7 |= F14;
    else
      data7 &= ~F14;
  }

  bool f15() const
  {
    return data7 & F15;
  }

  void setF15(bool on)
  {
    if(on)
      data7 |= F15;
    else
      data7 &= ~F15;
  }

  bool f16() const
  {
    return data7 & F16;
  }

  void setF16(bool on)
  {
    if(on)
      data7 |= F16;
    else
      data7 &= ~F16;
  }

  bool f17() const
  {
    return data7 & F17;
  }

  void setF17(bool on)
  {
    if(on)
      data7 |= F17;
    else
      data7 &= ~F17;
  }

  bool f18() const
  {
    return data7 & F18;
  }

  void setF18(bool on)
  {
    if(on)
      data7 |= F18;
    else
      data7 &= ~F18;
  }

  bool f19() const
  {
    return data7 & F19;
  }

  void setF19(bool on)
  {
    if(on)
      data7 |= F19;
    else
      data7 &= ~F19;
  }

  bool f20() const
  {
    return data3 & F20;
  }

  void setF20(bool on)
  {
    if(on)
      data3 |= F20;
    else
      data3 &= ~F20;
  }
};
static_assert(sizeof(LocoF13F20IMMLongAddress) == 11);

}

#endif
