/**
 * server/src/lua/classid.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "class.hpp"
#include "push.hpp"
#include "object.hpp"

#include "../board/board.hpp"
#include "../board/boardlist.hpp"

#include "../board/tile/rail/sensorrailtile.hpp"
#include "../board/tile/rail/straightrailtile.hpp"
#include "../board/tile/rail/cross45railtile.hpp"
#include "../board/tile/rail/tunnelrailtile.hpp"
#include "../board/tile/rail/blockrailtile.hpp"
#include "../board/tile/rail/curve45railtile.hpp"
#include "../board/tile/rail/bridge45rightrailtile.hpp"
#include "../board/tile/rail/cross90railtile.hpp"
#include "../board/tile/rail/bridge45leftrailtile.hpp"
#include "../board/tile/rail/bufferstoprailtile.hpp"
#include "../board/tile/rail/curve90railtile.hpp"
#include "../board/tile/rail/bridge90railtile.hpp"
#include "../board/tile/rail/turnout/turnoutsinglesliprailtile.hpp"
#include "../board/tile/rail/turnout/turnoutleft90railtile.hpp"
#include "../board/tile/rail/turnout/turnoutright45railtile.hpp"
#include "../board/tile/rail/turnout/turnoutleft45railtile.hpp"
#include "../board/tile/rail/turnout/turnoutright90railtile.hpp"
#include "../board/tile/rail/turnout/turnoutdoublesliprailtile.hpp"
#include "../board/tile/rail/turnout/turnoutwyerailtile.hpp"
#include "../board/tile/rail/turnout/turnoutleftcurvedrailtile.hpp"
#include "../board/tile/rail/turnout/turnout3wayrailtile.hpp"
#include "../board/tile/rail/turnout/turnoutrightcurvedrailtile.hpp"
#include "../board/tile/rail/signal/signal2aspectrailtile.hpp"
#include "../board/tile/rail/signal/signal3aspectrailtile.hpp"

#include "../clock/clock.hpp"


#include "../hardware/decoder/decoderfunction.hpp"
#include "../hardware/decoder/decoderlist.hpp"
#include "../hardware/decoder/decoder.hpp"
#include "../hardware/decoder/decoderfunctions.hpp"
#include "../hardware/decoder/decoderlisttablemodel.hpp"

#include "../hardware/input/input.hpp"
#include "../hardware/input/list/inputlist.hpp"
#include "../hardware/input/map/blockinputmap.hpp"
#include "../hardware/input/map/blockinputmapitem.hpp"

#include "../hardware/output/output.hpp"
#include "../hardware/output/list/outputlist.hpp"
#include "../hardware/output/map/outputmapoutputaction.hpp"
#include "../hardware/output/map/signaloutputmap.hpp"
#include "../hardware/output/map/turnoutoutputmap.hpp"
#include "../hardware/output/map/turnoutoutputmapitem.hpp"
#include "../hardware/output/map/signaloutputmapitem.hpp"

#include "../vehicle/rail/railvehiclelist.hpp"
#include "../vehicle/rail/locomotive.hpp"
#include "../vehicle/rail/freightcar.hpp"

#include "../train/train.hpp"
#include "../train/trainlist.hpp"

#include "../world/world.hpp"

namespace Lua {

static const char* metaTableName = "class";

using IsInstance = bool(*)(const ::ObjectPtr&);

template<class T>
static bool isInstance(const ::ObjectPtr& object)
{
  return dynamic_cast<T*>(object.get());
}

template<class T>
inline static void setField(lua_State* L, std::string_view key)
{
  static_assert(std::is_base_of_v<::Object, T>);
  push(L, key);
  *reinterpret_cast<IsInstance*>(lua_newuserdata(L, sizeof(IsInstance))) = isInstance<T>;
  luaL_newmetatable(L, metaTableName);
  lua_setmetatable(L, -2);
  lua_settable(L, -3);
}

void Class::registerValues(lua_State* L)
{
  assert(lua_istable(L, -1));

  setField<Board>(L, "BOARD");
  setField<BoardList>(L, "BOARD_LIST");

  setField<StraightRailTile>(L, "STRAIGHT_RAIL_TILE");
  setField<TunnelRailTile>(L, "TUNNEL_RAIL_TILE");
  setField<BufferStopRailTile>(L, "BUFFER_STOP_RAIL_TILE");
  setField<Curve45RailTile>(L, "CURVE_45_RAIL_TILE");
  setField<Curve90RailTile>(L, "CURVE_90_RAIL_TILE");
  setField<Cross45RailTile>(L, "CROSS_45_RAIL_TILE");
  setField<Cross90RailTile>(L, "CROSS_90_RAIL_TILE");
  setField<Bridge45RightRailTile>(L, "BRIDGE_45_RIGHT_RAIL_TILE");
  setField<Bridge45LeftRailTile>(L, "BRIDGE_45_LEFT_RAIL_TILE");
  setField<Bridge90RailTile>(L, "BRIDGE_90_RAIL_TILE");
  setField<TurnoutSingleSlipRailTile>(L, "TURNOUT_SINGLE_SLIP_RAIL_TILE");
  setField<TurnoutLeft90RailTile>(L, "TURNOUT_LEFT_90_RAIL_TILE");
  setField<TurnoutRight45RailTile>(L, "TURNOUT_RIGHT_45_RAIL_TILE");
  setField<TurnoutLeft45RailTile>(L, "TURNOUT_LEFT_45_RAIL_TILE");
  setField<TurnoutRight90RailTile>(L, "TURNOUT_RIGHT_90_RAIL_TILE");
  setField<TurnoutDoubleSlipRailTile>(L, "TURNOUT_DOUBLE_SLIP_RAIL_TILE");
  setField<TurnoutWyeRailTile>(L, "TURNOUT_WYE_RAIL_TILE");
  setField<TurnoutLeftCurvedRailTile>(L, "TURNOUT__RAIL_TILE");
  setField<Turnout3WayRailTile>(L, "TURNOUT_3WAY_RAIL_TILE");
  setField<TurnoutRightCurvedRailTile>(L, "TURNOUT_RIGHT_CURVED_RAIL_TILE");
  setField<Signal2AspectRailTile>(L, "SIGNAL_2_ASPECT_RAIL_TILE");
  setField<Signal3AspectRailTile>(L, "SIGNAL_3_ASPECT_RAIL_TILE");
  setField<SensorRailTile>(L, "SENSOR_RAIL_TILE");
  setField<BlockRailTile>(L, "BLOCK_RAIL_TILE");

  setField<Clock>(L, "CLOCK");

  setField<DecoderFunction>(L, "DECODER_FUNCTION");
  setField<DecoderList>(L, "DECODER_LIST");
  setField<Decoder>(L, "DECODER");
  setField<DecoderFunctions>(L, "DECODER_FUNCTIONS");

  setField<Input>(L, "INPUT");
  setField<InputList>(L, "INPUT_LIST");

  setField<Output>(L, "OUTPUT");
  setField<OutputList>(L, "OUTPUT_LIST");

  setField<RailVehicleList>(L, "RAIL_VEHICLE_LIST");
  setField<Locomotive>(L, "LOCOMOTIVE");
  setField<FreightCar>(L, "FREIGHT_CAR");

  setField<Train>(L, "TRAIN");
  setField<TrainList>(L, "TRAIN_LIST");

  setField<World>(L, "WORLD");
}

int Class::isInstance(lua_State* L)
{
  if(lua_gettop(L) != 2)
    errorExpectedNArgumentsGotN(L, 2, lua_gettop(L));

  push(L, (*reinterpret_cast<IsInstance*>(luaL_checkudata(L, 2, metaTableName)))(Object::test(L, 1)));

  return 1;
}

}
