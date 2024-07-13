/**
 * shared/src/traintastic/board/tileid.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2024 Reinder Feenstra
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
#include "../enum/enum.hpp"

enum class TileId : uint16_t // 10 bit
{
  None = 0,
  RailStraight = 1,
  RailCurve45 = 2,
  RailCurve90 = 3,
  RailCross45 = 4,
  RailCross90 = 5,
  RailTurnoutLeft45 = 6,
  RailTurnoutRight45 = 7,
  RailTurnoutWye = 8,
  RailTurnout3Way = 9,
  RailTurnoutSingleSlip = 10,
  RailTurnoutDoubleSlip = 11,
  RailSignal2Aspect = 12,
  RailSignal3Aspect = 13,
  RailBufferStop = 14,
  RailSensor = 15,
  RailBlock = 16,
  RailTurnoutLeft90 = 17,
  RailTurnoutRight90 = 18,
  RailTurnoutLeftCurved = 19,
  RailTurnoutRightCurved = 20,
  RailBridge45Left = 21,
  RailBridge45Right = 22,
  RailBridge90 = 23,
  RailTunnel = 24,
  RailOneWay = 25,
  RailDirectionControl = 26,
  PushButton = 27,
  RailLink = 28,
  RailDecoupler = 29,
  RailNXButton = 30,
  Label = 31,
  Switch = 32,

  ReservedForFutureExpension = 1023
};

constexpr bool isRail(TileId id)
{
  switch(id)
  {
    case TileId::RailStraight:
    case TileId::RailCurve45:
    case TileId::RailCurve90:
    case TileId::RailCross45:
    case TileId::RailCross90:
    case TileId::RailTurnoutLeft45:
    case TileId::RailTurnoutRight45:
    case TileId::RailTurnoutWye:
    case TileId::RailTurnout3Way:
    case TileId::RailTurnoutSingleSlip:
    case TileId::RailTurnoutDoubleSlip:
    case TileId::RailSignal2Aspect:
    case TileId::RailSignal3Aspect:
    case TileId::RailBufferStop:
    case TileId::RailSensor:
    case TileId::RailBlock:
    case TileId::RailTurnoutLeft90:
    case TileId::RailTurnoutRight90:
    case TileId::RailTurnoutLeftCurved:
    case TileId::RailTurnoutRightCurved:
    case TileId::RailBridge45Left:
    case TileId::RailBridge45Right:
    case TileId::RailBridge90:
    case TileId::RailTunnel:
    case TileId::RailOneWay:
    case TileId::RailDirectionControl:
    case TileId::RailLink:
    case TileId::RailDecoupler:
    case TileId::RailNXButton:
      return true;

    default:
      break;
  }
  return false;
}

constexpr bool isRailCross(TileId id)
{
  switch(id)
  {
    case TileId::RailCross45:
    case TileId::RailCross90:
      return true;

    default:
      break;
  }
  return false;
}

constexpr bool isRailBridge(TileId id)
{
  switch(id)
  {
    case TileId::RailBridge45Left:
    case TileId::RailBridge45Right:
    case TileId::RailBridge90:
      return true;

    default:
      break;
  }
  return false;
}

constexpr bool isRailTurnout(TileId id)
{
  switch(id)
  {
    case TileId::RailTurnoutLeft45:
    case TileId::RailTurnoutLeft90:
    case TileId::RailTurnoutLeftCurved:
    case TileId::RailTurnoutRight45:
    case TileId::RailTurnoutRight90:
    case TileId::RailTurnoutRightCurved:
    case TileId::RailTurnoutWye:
    case TileId::RailTurnout3Way:
    case TileId::RailTurnoutSingleSlip:
    case TileId::RailTurnoutDoubleSlip:
      return true;

    default:
      break;
  }
  return false;
}

constexpr bool isRailSignal(TileId id)
{
  switch(id)
  {
    case TileId::RailSignal2Aspect:
    case TileId::RailSignal3Aspect:
      return true;

    default:
      break;
  }
  return false;
}

constexpr bool isActive(TileId id)
{
  switch(id)
  {
    case TileId::RailTurnoutLeft45:
    case TileId::RailTurnoutLeft90:
    case TileId::RailTurnoutLeftCurved:
    case TileId::RailTurnoutRight45:
    case TileId::RailTurnoutRight90:
    case TileId::RailTurnoutRightCurved:
    case TileId::RailTurnoutWye:
    case TileId::RailTurnout3Way:
    case TileId::RailTurnoutSingleSlip:
    case TileId::RailTurnoutDoubleSlip:
    case TileId::RailSignal2Aspect:
    case TileId::RailSignal3Aspect:
    case TileId::RailSensor:
    case TileId::RailBlock:
    case TileId::RailDirectionControl:
    case TileId::PushButton:
    case TileId::RailLink:
    case TileId::RailDecoupler:
    case TileId::RailNXButton:
    case TileId::Label:
    case TileId::Switch:
      return true;

    default:
      break;
  }
  return false;
}

TRAINTASTIC_ENUM(TileId, "tile_id", 0, {}); // no values defined as we don't need them (at the moment).

#endif
