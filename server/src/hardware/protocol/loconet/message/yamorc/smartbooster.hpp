/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_YAMORC_SMARTBOOSTER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_YAMORC_SMARTBOOSTER_HPP

#include "../../messages.hpp"
#include "../../../../../utils/bit.hpp"

namespace LocoNet::YaMoRC {

struct SmartBooster : MultiSense
{
  enum class Payload : uint8_t
  {
    Status = 0,
    Temperature = 1,
    Load = 2,
    CurrentUnits = 3,
    OutputVoltage = 4,
    InputVoltage = 5,
    MaxCurrentUnits = 6,
    Source = 7,
    CurrentScale = 8,
    Control = 15,

    RequestFlag = 0x10,
    RequestStatus = RequestFlag | Status,
    RequestTemperature = RequestFlag | Temperature,
    RequestLoad = RequestFlag | Load,
    RequestCurrentUnits = RequestFlag | CurrentUnits,
    RequestOutputVoltage = RequestFlag | OutputVoltage,
    RequestInputVoltage = RequestFlag | InputVoltage,
    RequestMaxCurrentUnits = RequestFlag | MaxCurrentUnits,
    RequestSource = RequestFlag | Source,
    RequestCurrentScale = RequestFlag | CurrentScale,

    RequestAll = 0x1F,
  };

  static bool match(const Message& message)
  {
    return
      (message.opCode == OPC_MULTI_SENSE) &&
      (static_cast<const MultiSense&>(message).data1 & 0x6E) == 0x60;
  }

  bool isWrite() const
  {
    return getBit<4>(data1);
  }

  void setWrite(bool value)
  {
    setBit<4>(data1, value);
  }

  uint8_t boosterId() const
  {
    return (data2 | (data1 << 7));
  }

  void setBoosterId(uint8_t value)
  {
    setBit<0>(data1, getBit<7>(value));
    data2 = value & 0x7F;
  }

  Payload payload() const
  {
    return static_cast<Payload>(data3 & 0x3F);
  }

  void setPayload(Payload value)
  {
    data3 &= 0xC0;
    data3 |= static_cast<uint8_t>(value) & 0x3F;
  }

  uint8_t value() const
  {
    return ((data3 << 1) & 0x80) | data4;
  }

protected:
  SmartBooster(uint8_t id, bool write)
  {
    data1 = 0x60;
    data2 = 0x00;
    data3 = 0x00;
    data4 = 0x00;
    setBoosterId(id);
    setWrite(write);
  }
};
static_assert(sizeof(SmartBooster) == 6);

struct SmartBoosterSetUnsolicitedEvents : SmartBooster
{
  SmartBoosterSetUnsolicitedEvents(uint8_t id, bool enable)
    : SmartBooster(id, true)
  {
    setPayload(Payload::Control);
    data4 = enable ? 0x15 : 0x05;
  }
};

struct SmartBoosterRequestAll : SmartBooster
{
  SmartBoosterRequestAll(uint8_t id)
    : SmartBooster(id, true)
  {
    setPayload(Payload::RequestAll);
  }
};

struct SmartBoosterStatus : SmartBooster
{
  bool unsolicited() const
  {
    return getBit<5>(data4);
  }

  bool inputSignalPresent() const
  {
    return getBit<4>(data4);
  }

  bool inputVoltagePresent() const
  {
    return getBit<3>(data4);
  }

  bool shortCircuit() const
  {
    return getBit<2>(data4);
  }

  bool outputInverted() const
  {
    return getBit<1>(data4);
  }

  bool outputOn() const
  {
    return getBit<0>(data4);
  }
};
static_assert(sizeof(SmartBoosterStatus) == 6);

struct SmartBoosterSource : SmartBooster
{
  enum class Source : uint8_t
  {
    Auto = 0,
    LocoNet = 1,
    CDE = 2,
    BBUS = 3,
    Internal = 4,
  };

  Source source() const
  {
    return static_cast<Source>(value());
  }
};
static_assert(sizeof(SmartBoosterSource) == 6);

}

#endif
