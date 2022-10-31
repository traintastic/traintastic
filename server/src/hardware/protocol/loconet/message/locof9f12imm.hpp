/**
 * server/src/hardware/protocol/loconet/message/locof9f12imm.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_LOCOF9F12IMM_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_LOCOF9F12IMM_HPP

#include "message.hpp"
#include "../checksum.hpp"
#include "../signature.hpp"

namespace LocoNet {

struct LocoF9F12IMMShortAddress : Message
{
  static constexpr std::array<SignatureByte, 10> signature =
  {{
    {OPC_IMM_PACKET},
    {11, 0xFF},
    {0x7F, 0xFF},
    {0x24, 0xFF},
    {0x02, 0x82},
    {},
    {0x20, 0xA0},
    {0x00, 0xFF},
    {0x00, 0xFF},
    {0x00, 0xFF}
  }};

  static constexpr uint8_t F9 = 0x01;
  static constexpr uint8_t F10 = 0x02;
  static constexpr uint8_t F11 = 0x04;
  static constexpr uint8_t F12 = 0x08;

  uint8_t len = 11;
  uint8_t data1 = 0x7F;
  uint8_t data2 = 0x24;
  uint8_t data3 = 0x22;
  uint8_t data4 = 0;
  uint8_t data5 = 0x20;
  uint8_t data6 = 0x00;
  uint8_t data7 = 0x00;
  uint8_t data8 = 0x00;
  uint8_t checksum;

  LocoF9F12IMMShortAddress(uint8_t shortAddress, bool f9on, bool f10on, bool f11on, bool f12on)
    : Message(OPC_IMM_PACKET)
  {
    setAddress(shortAddress);
    setF9(f9on);
    setF10(f10on);
    setF11(f11on);
    setF12(f12on);
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
    assert(n >= 9 && n <= 12);
    return (data5 & (1 << (n - 9)));
  }

  bool f9() const
  {
    return data5 & F9;
  }

  void setF9(bool on)
  {
    if(on)
      data5 |= F9;
    else
      data5 &= ~F9;
  }

  bool f10() const
  {
    return data5 & F10;
  }

  void setF10(bool on)
  {
    if(on)
      data5 |= F10;
    else
      data5 &= ~F10;
  }

  bool f11() const
  {
    return data5 & F11;
  }

  void setF11(bool on)
  {
    if(on)
      data5 |= F11;
    else
      data5 &= ~F11;
  }

  bool f12() const
  {
    return data5 & F12;
  }

  void setF12(bool on)
  {
    if(on)
      data5 |= F12;
    else
      data5 &= ~F12;
  }
};
static_assert(sizeof(LocoF9F12IMMShortAddress) == 11);

struct LocoF9F12IMMLongAddress : Message
{
  static constexpr std::array<SignatureByte, 10> signature =
  {{
    {OPC_IMM_PACKET},
    {11, 0xFF},
    {0x7F, 0xFF},
    {0x34, 0xFF},
    {0x05, 0xD5},
    {0x40, 0x40},
    {},
    {0x20, 0xF0},
    {0x00, 0xFF},
    {0x00, 0xFF}
  }};

  static constexpr uint8_t F9 = 0x01;
  static constexpr uint8_t F10 = 0x02;
  static constexpr uint8_t F11 = 0x04;
  static constexpr uint8_t F12 = 0x08;

  uint8_t len = 11;
  uint8_t data1 = 0x7F;
  uint8_t data2 = 0x34;
  uint8_t data3 = 0x25;
  uint8_t data4 = 0x40;
  uint8_t data5 = 0;
  uint8_t data6 = 0x20;
  uint8_t data7 = 0x00;
  uint8_t data8 = 0x00;
  uint8_t checksum;

  LocoF9F12IMMLongAddress(uint16_t longAddress, bool f9on, bool f10on, bool f11on, bool f12on)
    : Message(OPC_IMM_PACKET)
  {
    setAddress(longAddress);
    setF9(f9on);
    setF10(f10on);
    setF11(f11on);
    setF12(f12on);
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
    assert(n >= 9 && n <= 12);
    return (data6 & (1 << (n - 9)));
  }

  bool f9() const
  {
    return data6 & F9;
  }

  void setF9(bool on)
  {
    if(on)
      data6 |= F9;
    else
      data6 &= ~F9;
  }

  bool f10() const
  {
    return data6 & F10;
  }

  void setF10(bool on)
  {
    if(on)
      data6 |= F10;
    else
      data6 &= ~F10;
  }

  bool f11() const
  {
    return data6 & F11;
  }

  void setF11(bool on)
  {
    if(on)
      data6 |= F11;
    else
      data6 &= ~F11;
  }

  bool f12() const
  {
    return data6 & F12;
  }

  void setF12(bool on)
  {
    if(on)
      data6 |= F12;
    else
      data6 &= ~F12;
  }
};
static_assert(sizeof(LocoF9F12IMMLongAddress) == 11);

}

#endif
