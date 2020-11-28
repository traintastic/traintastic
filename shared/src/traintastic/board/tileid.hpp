/**
 * shared/src/traintastic/board/tileid.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_BOARD_TILEID_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_BOARD_TILEID_HPP

#include <cstdint>

enum class TileId : uint16_t // 10 bit
{
  None = 0,
  RailStraight = 1,
  RailCurve45 = 2,
  RailCurve90 = 3,
  RailCross45 = 4,
  RailCross90 = 5,
  RailTurnoutLeft = 6,
  RailTurnoutRight = 7,
  RailTurnoutWye = 8,
  RailTurnout3Way = 9,
  RailTurnoutSingleSlip = 10,
  RailTurnoutDoubleSlip = 11,
  RailSignal2Aspect = 12,
  RailSignal3Aspect = 13,

  ReservedForFutureExpension = 1023
};

#endif
