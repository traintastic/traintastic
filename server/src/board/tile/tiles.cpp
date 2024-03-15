/**
 * server/src/board/tile/tiles.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2023 Reinder Feenstra
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

#include "tiles.hpp"
#include "rail/decouplerrailtile.hpp"
#include "rail/linkrailtile.hpp"
#include "rail/nxbuttonrailtile.hpp"
#include "../../utils/ifclassidcreate.hpp"
#include "../../world/world.hpp"

std::shared_ptr<Tile> Tiles::create(World& world, std::string_view classId, std::string_view id)
{
  IF_CLASSID_CREATE(StraightRailTile)
  IF_CLASSID_CREATE(OneWayRailTile)
  IF_CLASSID_CREATE(DirectionControlRailTile)
  IF_CLASSID_CREATE(Curve45RailTile)
  IF_CLASSID_CREATE(Curve90RailTile)
  IF_CLASSID_CREATE(Bridge45LeftRailTile)
  IF_CLASSID_CREATE(Bridge45RightRailTile)
  IF_CLASSID_CREATE(Bridge90RailTile)
  IF_CLASSID_CREATE(Cross45RailTile)
  IF_CLASSID_CREATE(Cross90RailTile)
  IF_CLASSID_CREATE(TurnoutLeft45RailTile)
  IF_CLASSID_CREATE(TurnoutLeft90RailTile)
  IF_CLASSID_CREATE(TurnoutLeftCurvedRailTile)
  IF_CLASSID_CREATE(TurnoutRight45RailTile)
  IF_CLASSID_CREATE(TurnoutRight90RailTile)
  IF_CLASSID_CREATE(TurnoutRightCurvedRailTile)
  IF_CLASSID_CREATE(TurnoutWyeRailTile)
  IF_CLASSID_CREATE(Turnout3WayRailTile)
  IF_CLASSID_CREATE(TurnoutSingleSlipRailTile)
  IF_CLASSID_CREATE(TurnoutDoubleSlipRailTile)
  IF_CLASSID_CREATE(Signal2AspectRailTile)
  IF_CLASSID_CREATE(Signal3AspectRailTile)
  IF_CLASSID_CREATE(BufferStopRailTile)
  IF_CLASSID_CREATE(SensorRailTile)
  IF_CLASSID_CREATE(BlockRailTile)
  IF_CLASSID_CREATE(TunnelRailTile)
  IF_CLASSID_CREATE(LinkRailTile)
  IF_CLASSID_CREATE(PushButtonTile)
  IF_CLASSID_CREATE(DecouplerRailTile)
  IF_CLASSID_CREATE(NXButtonRailTile)
  return std::shared_ptr<Tile>();
}

const std::vector<Tiles::Info>& Tiles::getInfo()
{
  static constexpr uint8_t rotateNone = 0x01; //!< only 0 deg
  static constexpr uint8_t rotate0and45 = 0x03; //!< only 0 or 45 deg
  static constexpr uint8_t rotate0and90 = 0x05; //!< only 0 or 90 deg
  static constexpr uint8_t rotateHalf = 0x0F; //!< only 0, 45, 90 or 135 deg
  static constexpr uint8_t rotateFull = 0xFF;

  static constexpr std::string_view straight = "tile_menu:straight";
  static constexpr std::string_view curve = "tile_menu:curve";
  static constexpr std::string_view crossAndBridge = "tile_menu:cross_and_bridge";
  static constexpr std::string_view turnout = "tile_menu:turnout";
  static constexpr std::string_view blockAndSensor = "tile_menu:block_and_sensor";
  static constexpr std::string_view signal = "tile_menu:signal";
  static constexpr std::string_view miscellaneous = "tile_menu:miscellaneous";

  static const std::vector<Tiles::Info> info{{
    Info{StraightRailTile::classId, TileId::RailStraight, rotateHalf, {straight}},
    Info{BufferStopRailTile::classId, TileId::RailBufferStop, rotateFull, {straight}},
    Info{TunnelRailTile::classId, TileId::RailTunnel, rotateFull, {straight}},
    Info{OneWayRailTile::classId, TileId::RailOneWay, rotateFull, {straight}},
    Info{DirectionControlRailTile::classId, TileId::RailDirectionControl, rotateHalf, {straight}},
    Info{LinkRailTile::classId, TileId::RailLink, rotateFull, {straight}},
    Info{DecouplerRailTile::classId, TileId::RailDecoupler, rotateHalf, {straight}},
    Info{NXButtonRailTile::classId, TileId::RailNXButton, rotateHalf, {straight}},

    Info{Curve45RailTile::classId, TileId::RailCurve45, rotateFull, {curve}},
    Info{Curve90RailTile::classId, TileId::RailCurve90, rotateFull, {curve}},

    Info{Cross45RailTile::classId, TileId::RailCross45, rotateHalf, {crossAndBridge}},
    Info{Cross90RailTile::classId, TileId::RailCross90, rotate0and45, {crossAndBridge}},
    Info{Bridge45LeftRailTile::classId, TileId::RailBridge45Left, rotateHalf, {crossAndBridge}},
    Info{Bridge45RightRailTile::classId, TileId::RailBridge45Right, rotateHalf, {crossAndBridge}},
    Info{Bridge90RailTile::classId, TileId::RailBridge90, rotateHalf, {crossAndBridge}},

    Info{TurnoutLeft45RailTile::classId, TileId::RailTurnoutLeft45, rotateFull, {turnout}},
    Info{TurnoutLeft90RailTile::classId, TileId::RailTurnoutLeft90, rotateFull, {turnout}},
    Info{TurnoutLeftCurvedRailTile::classId, TileId::RailTurnoutLeftCurved, rotateFull, {turnout}},
    Info{TurnoutRight45RailTile::classId, TileId::RailTurnoutRight45, rotateFull, {turnout}},
    Info{TurnoutRight90RailTile::classId, TileId::RailTurnoutRight90, rotateFull, {turnout}},
    Info{TurnoutRightCurvedRailTile::classId, TileId::RailTurnoutRightCurved, rotateFull, {turnout}},
    Info{TurnoutWyeRailTile::classId, TileId::RailTurnoutWye, rotateFull, {turnout}},
    Info{Turnout3WayRailTile::classId, TileId::RailTurnout3Way, rotateFull, {turnout}},
    Info{TurnoutSingleSlipRailTile::classId, TileId::RailTurnoutSingleSlip, rotateFull, {turnout}},
    Info{TurnoutDoubleSlipRailTile::classId, TileId::RailTurnoutDoubleSlip, rotateFull, {turnout}},

    Info{BlockRailTile::classId, TileId::RailBlock, rotate0and90, {blockAndSensor}},
    Info{SensorRailTile::classId, TileId::RailSensor, rotateHalf, {blockAndSensor}},

    Info{Signal2AspectRailTile::classId, TileId::RailSignal2Aspect, rotateFull, {signal}},
    Info{Signal3AspectRailTile::classId, TileId::RailSignal3Aspect, rotateFull, {signal}},

    Info{PushButtonTile::classId, TileId::PushButton, rotateNone, {miscellaneous}},
  }};

  return info;
}

bool Tiles::canUpgradeStraightRail(std::string_view classId)
{
  return
    (classId == OneWayRailTile::classId) ||
    (classId == DirectionControlRailTile::classId) ||
    (classId == TunnelRailTile::classId) ||
    (classId == BlockRailTile::classId) ||
    (classId == SensorRailTile::classId) ||
    (classId == Signal2AspectRailTile::classId) ||
    (classId == Signal3AspectRailTile::classId) ||
    (classId == DecouplerRailTile::classId) ||
    (classId == NXButtonRailTile::classId);
}
