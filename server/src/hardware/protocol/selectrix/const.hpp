/**
 * server/src/hardware/protocol/selectrix/const.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023,2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_CONST_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_CONST_HPP

#include <cstdint>

namespace Selectrix {

namespace Address
{
  constexpr uint8_t locomotiveMax = 103;
  constexpr uint8_t selectSXBus = 126;
  constexpr uint8_t trackPower = 127;

  constexpr uint8_t max = 127;
  constexpr uint8_t writeFlag = 0x80;
}

namespace TrackPower
{
  constexpr uint8_t on = 0x80;
  constexpr uint8_t off = 0x00;
}

namespace Locomotive
{
  constexpr uint8_t speedStepMax = 31;
  constexpr uint8_t speedMask = 0x1F;
  constexpr uint8_t directionMask = 0x20;
  constexpr uint8_t directionReverse = 0x20;
  constexpr uint8_t directionForward = 0x00;
  constexpr uint8_t f0 = 0x40;
  constexpr uint8_t f1 = 0x80;
}

}

#endif
