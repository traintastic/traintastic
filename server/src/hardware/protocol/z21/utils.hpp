/**
 * server/src/hardware/protocol/z21/utils.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_UTILS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_UTILS_HPP

#include <cstdint>
#include <traintastic/enum/direction.hpp>

namespace Z21::Utils {

inline constexpr uint8_t directionFlag = 0x80;

constexpr Direction getDirection(uint8_t db)
{
  return (db & directionFlag) ? Direction::Forward : Direction::Reverse;
}

constexpr void setDirection(uint8_t& db, Direction direction)
{
  if(direction == Direction::Forward)
    db |= directionFlag;
  else
    db &= ~directionFlag;
}

constexpr bool isEmergencyStop(uint8_t db, uint8_t speedSteps)
{
  switch(speedSteps)
  {
    case 126:
      return (db & 0x7F) == 0x01;

    case 28:
      return (db & 0x1F) == 0x01 || (db & 0x1F) == 0x11;

    case 14:
      return (db & 0x0F) == 0x01;
  }
  return true;
}

constexpr void setEmergencyStop(uint8_t& db)
{
  db = (db & directionFlag) | 0x01; // preserve direction flag
}

constexpr uint8_t getSpeedStep(uint8_t db, uint8_t speedSteps)
{
  if(isEmergencyStop(db, speedSteps))
    return 0;

  switch(speedSteps)
  {
    case 126:
      db &= 0x7F;
      break;

    case 28:
      db = ((db & 0x0F) << 1) | ((db & 0x10) >> 4); //! @todo check
      if(db >= 3)
          db -= 2;
      break;

    case 14:
      db &= 0x0F;
      break;

    default:
      return 0;
  }
  return db >= 1 ? db - 1 : 0; // step 1 = EStop
}

constexpr void setSpeedStep(uint8_t& db, uint8_t speedSteps, uint8_t speedStep)
{
  db &= directionFlag; // preserve direction flag
  if(++speedStep > 1)
    switch(speedSteps)
    {
      case 126:
        db |= speedStep & 0x7F;
        break;

      case 28:
        speedStep += 2;
        db |= ((speedStep >> 1) & 0x0F) | ((speedStep & 0x01) << 4);
        break;

      case 14:
        db |= speedStep & 0x0F;
        break;
    }
}

constexpr uint8_t toBCD(uint8_t value)
{
  return ((value / 10) << 4) | (value % 10);
}

constexpr uint8_t fromBCD(uint8_t value)
{
  return ((value >> 4) * 10) + (value & 0x0F);
}

}

#endif
