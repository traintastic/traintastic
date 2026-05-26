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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSCOMMANDSTATIONMESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSCOMMANDSTATIONMESSAGES_HPP

#include "cbusmessage.hpp"
#include "../../../../utils/bit.hpp"

namespace CBUS {

struct CommandStationStatusReport : NodeMessage
{
  static constexpr uint8_t hardwareErrorBit = 0;
  static constexpr uint8_t trackErrorBit = 1;
  static constexpr uint8_t trackOnBit = 2;
  static constexpr uint8_t busOnBit = 3;
  static constexpr uint8_t eStopAllBit = 4;
  static constexpr uint8_t resetDoneBit = 5;
  static constexpr uint8_t serviceModeBit = 6;

  uint8_t csNum;
  uint8_t flags;
  uint8_t majorRev;
  uint8_t minorRev;
  uint8_t build;

  CommandStationStatusReport(uint16_t nodeNumber_,
        bool hardwareError_, bool trackError_, bool trackOn_, bool busOn_,
        bool eStopAll_, bool resetDone_, bool serviceMode_,
        uint8_t majorRev_, uint8_t minorRev_, uint8_t build_)
    : NodeMessage(OpCode::STAT, nodeNumber_)
    , csNum{0} // For future expansion, must be zero now
    , flags{0}
    , majorRev{majorRev_}
    , minorRev{minorRev_}
    , build{build_}
  {
    setBit<hardwareErrorBit>(flags, hardwareError_);
    setBit<trackErrorBit>(flags, trackError_);
    setBit<trackOnBit>(flags, trackOn_);
    setBit<busOnBit>(flags, busOn_);
    setBit<eStopAllBit>(flags, eStopAll_);
    setBit<resetDoneBit>(flags, resetDone_);
    setBit<serviceModeBit>(flags, serviceMode_);
  }

  bool hardwareError() const
  {
    return getBit<hardwareErrorBit>(flags);
  }

  bool trackError() const
  {
    return getBit<trackErrorBit>(flags);
  }

  bool trackOn() const
  {
    return getBit<trackOnBit>(flags);
  }

  bool busOn() const
  {
    return getBit<busOnBit>(flags);
  }

  bool eStopAll() const
  {
    return getBit<eStopAllBit>(flags);
  }

  bool resetDone() const
  {
    return getBit<resetDoneBit>(flags);
  }

  bool serviceMode() const
  {
    return getBit<serviceModeBit>(flags);
  }
};

}

#endif
