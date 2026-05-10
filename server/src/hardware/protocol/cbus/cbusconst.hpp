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

namespace CBUS {

constexpr uint16_t defaultTCPPort = 5550;

constexpr uint8_t canIdMin = 1;
constexpr uint8_t canIdMax = 127;

constexpr uint8_t engineFunctionMax = 28;

struct CanId
{
  static constexpr uint8_t CANUSB = 0x7C;
  static constexpr uint8_t CANEther = 0x7D;
};

struct NodeNumber
{
  static constexpr uint16_t CANCMD = 0xFFFE;
  static constexpr uint16_t CANCAB = 0xFFFF;
};

}

#endif
