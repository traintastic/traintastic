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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSCONST_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSCONST_HPP

#include <cstdint>
#include <utility>

namespace CBUS {

constexpr uint16_t defaultTCPPort = 5550;

constexpr uint8_t engineFunctionMax = 28;

struct CanId
{
  // see: https://github.com/SvenRosvall/VLCB-defs/blob/main/vlcbdefs.csv

  static constexpr uint8_t CANCMD      = 0x72; // (114) Fixed CANID for CANCMD or CANCSB
  static constexpr uint8_t TRAINTASTIC = 0x7A; // (122) !!! Not yet assigned, pending. !!!
  static constexpr uint8_t MMC         = 0x7B; // (123) Default CANID used by MMC.
  static constexpr uint8_t CANUSB      = 0x7C; // (124) Fixed CANID for CANUSB, although in current firmware it may just use the CANID from the sending software
  static constexpr uint8_t FCU         = 0x7D; // (125) Default CANID used by FCU. Can be changed in settings. Note some interface modules may substitute their own CANID.
  static constexpr uint8_t JMRI        = 0x7E; // (126) Default CANID used by JMRI. Can be changed in connection preferences. Note some interface modules may substitute their own CANID.
  static constexpr uint8_t CANEther    = 0x7F; // (127) Default fixed CANID for CANEther (can be changed by modifying NV2). Note CANEther inserts its own CANID on all packets transmitted on CAN

  static constexpr uint8_t min()
  {
    return 1;
  }

  static constexpr uint8_t max()
  {
    return 127;
  }

  static constexpr std::pair<uint8_t, uint8_t> range()
  {
    return {min(), max()};
  }
};

struct NodeNumber
{
  static constexpr uint16_t CANCMD = 0xFFFE;
  static constexpr uint16_t CANCAB = 0xFFFF;
};

}

#endif
