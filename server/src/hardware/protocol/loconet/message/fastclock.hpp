/**
 * server/src/hardware/protocol/loconet/message/fastclock.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_FASTCLOCK_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_FASTCLOCK_HPP

#include "slot.hpp"
#include <cassert>
#include "../checksum.hpp"

namespace LocoNet {

struct FastClockSlotData : SlotDataBase
{
  static constexpr uint8_t CLK_CNTRL_VALID = 0x40;

  uint8_t clk_rate = 0;
  uint8_t frac_minsl = 0;
  uint8_t frac_minsh = 0;
  uint8_t mins_60 = 0;
  uint8_t trk = 0;
  uint8_t hrs_24 = 0;
  uint8_t days = 0;
  uint8_t clk_cntrl = 0;
  uint8_t id1 = 0;
  uint8_t id2 = 0;
  uint8_t checksum;

  FastClockSlotData(OpCode opcode_)
    : SlotDataBase(opcode_, SLOT_FAST_CLOCK)
    , checksum{calcChecksum(*this)}
  {
  }

  uint8_t hour() const
  {
    return (24 - (((256 - hrs_24) & 0x7F) % 24)) % 24;
  }

  void setHour(uint8_t value)
  {
    assert(value < 24);
    hrs_24 = (256 - (24 - value)) & 0x7F;
  }

  uint8_t minute() const
  {
    return (60 - (((256 - mins_60) & 0x7F) % 60)) % 60;
  }

  void setMinute(uint8_t value)
  {
    assert(value < 60);
    mins_60 = (256 - (60 - value)) & 0x7F;
  }

  bool valid() const
  {
    return clk_cntrl & CLK_CNTRL_VALID;
  }

  void setValid(bool value)
  {
    if(value)
      clk_cntrl |= CLK_CNTRL_VALID;
    else
      clk_cntrl &= ~CLK_CNTRL_VALID;
  }

  uint16_t id() const
  {
    return (static_cast<uint16_t>(id2) << 7) | id1;
  }

  void setId(uint16_t value)
  {
    assert(value <= 0x3FFF);
    id2 = (value >> 7) & 0x7F;
    id1 = value & 0x7F;
  }
};
static_assert(sizeof(FastClockSlotData) == 14);

struct FastClockSlotReadData : FastClockSlotData
{
  FastClockSlotReadData()
    : FastClockSlotData(OPC_SL_RD_DATA)
  {
  }
};
static_assert(sizeof(FastClockSlotReadData) == 14);

struct FastClockSlotWriteData : FastClockSlotData
{
  FastClockSlotWriteData()
    : FastClockSlotData(OPC_WR_SL_DATA)
  {
  }
};
static_assert(sizeof(FastClockSlotWriteData) == 14);

}

#endif
