/**
 * server/src/hardware/protocol/loconet/message/multisenselong.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_MULTISENSELONG_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_MULTISENSELONG_HPP

#include "message.hpp"
#include <traintastic/enum/direction.hpp>
#include "../../railcom/appdynid.hpp"

namespace LocoNet {

struct MultiSenseLong : Message
{
  enum class Code
  {
    ReleaseTransponder = 0x00,
    DetectTransponder = 0x20,
    RailComAppDyn = 0x40,
    Reserved = 0x60,
  };

  uint8_t len;
  uint8_t data1;
  uint8_t data2;
  uint8_t data3;
  uint8_t data4;
  uint8_t data5;
  uint8_t data6;
  uint8_t checksum;

  MultiSenseLong() :
    Message(OPC_MULTI_SENSE_LONG),
    len{9}
  {
  }

  Code code() const
  {
    return static_cast<Code>(data1 & 0x60);
  }

  uint16_t sensorAddress() const
  {
    return (static_cast<uint16_t>(data1 & 0x1F) << 7) | (data2 & 0x7F);
  }

  uint16_t transponderAddress() const
  {
    if(isTransponderAddressLong())
      return (static_cast<uint16_t>(data3 & 0x7F) << 7) | (data4 & 0x7F);
    else
      return (data4 & 0x7F);
  }

  bool isTransponderAddressLong() const
  {
    return data3 != 0xFD;
  }

  Direction transponderDirection() const
  {
    return (data5 & 0x40) ? Direction::Forward : Direction::Reverse;
  }
};
static_assert(sizeof(MultiSenseLong) == 9);

struct MultiSenseLongRailComAppDyn : MultiSenseLong
{
  RailCom::AppDynId appDynId() const
  {
    return static_cast<RailCom::AppDynId>((data5 >> 1) & 0x1F);
  }

  uint8_t value() const
  {
    return ((data5 & 0x01) << 7) | data6;
  }
};
static_assert(sizeof(MultiSenseLongRailComAppDyn) == 9);

struct MultiSenseLongRailComAppDynActualSpeed : MultiSenseLongRailComAppDyn
{
  uint16_t actualSpeed() const
  {
    if(appDynId() == RailCom::AppDynId::ActualSpeed)
    {
      return value();
    }
    assert(appDynId() == RailCom::AppDynId::ActualSpeedHigh);
    return 0x100 + value();
  }
};
static_assert(sizeof(MultiSenseLongRailComAppDynActualSpeed) == 9);

}

constexpr std::string_view toString(LocoNet::MultiSenseLong::Code value)
{
  switch(value)
  {
    case LocoNet::MultiSenseLong::Code::ReleaseTransponder:
      return "Release";

    case LocoNet::MultiSenseLong::Code::DetectTransponder:
      return "Detect";

    case LocoNet::MultiSenseLong::Code::RailComAppDyn:
      return "RailComAppDyn";

    case LocoNet::MultiSenseLong::Code::Reserved:
      return "Reserved";
  }
  return {};
}

#endif
