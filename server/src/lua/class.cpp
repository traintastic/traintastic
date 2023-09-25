/**
 * server/src/lua/classid.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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
#include "test.hpp"
#include "checkarguments.hpp"
#include "sandbox.hpp"

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

#include "../hardware/interface/dccplusplusinterface.hpp"
#include "../hardware/interface/ecosinterface.hpp"
#include "../hardware/interface/hsi88.hpp"
#include "../hardware/interface/loconetinterface.hpp"
#include "../hardware/interface/marklincaninterface.hpp"
#include "../hardware/interface/traintasticdiyinterface.hpp"
#include "../hardware/interface/withrottleinterface.hpp"
#include "../hardware/interface/wlanmausinterface.hpp"
#include "../hardware/interface/xpressnetinterface.hpp"
#include "../hardware/interface/z21interface.hpp"

#include "../hardware/decoder/decoderfunction.hpp"
#include "../hardware/decoder/list/decoderlist.hpp"
#include "../hardware/decoder/decoder.hpp"
#include "../hardware/decoder/decoderfunctions.hpp"

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

#include "../hardware/identification/identification.hpp"
#include "../hardware/identification/list/identificationlist.hpp"

#include "../vehicle/rail/railvehiclelist.hpp"
#include "../vehicle/rail/locomotive.hpp"
#include "../vehicle/rail/freightwagon.hpp"

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
inline static void registerValue(lua_State* L, std::string_view key)
{
  static_assert(std::is_base_of_v<::Object, T>);

  // add to global class (used by Class::push)
  lua_getglobal(L, metaTableName);
  Lua::push(L, T::classId);
  *reinterpret_cast<IsInstance*>(lua_newuserdata(L, sizeof(IsInstance))) = isInstance<T>;
  if(luaL_newmetatable(L, metaTableName))
  {
    lua_pushcfunction(L, Class::__tostring);
    lua_setfield(L, -2, "__tostring");
  }
  lua_setmetatable(L, -2);
  lua_rawset(L, -3);
  lua_pop(L, 1);

  // add to sandbox class global:
  Lua::push(L, key);
  Class::push<T>(L);
  lua_settable(L, -3);
}

void Class::registerValues(lua_State* L)
{
  assert(lua_istable(L, -1)); // sandboxes global class

  lua_newtable(L); // global
  lua_setglobal(L, metaTableName);

  lua_pushcfunction(L, getClass);
  lua_setfield(L, -2, "get");

  registerValue<Board>(L, "BOARD");
  registerValue<BoardList>(L, "BOARD_LIST");

  registerValue<StraightRailTile>(L, "STRAIGHT_RAIL_TILE");
  registerValue<TunnelRailTile>(L, "TUNNEL_RAIL_TILE");
  registerValue<BufferStopRailTile>(L, "BUFFER_STOP_RAIL_TILE");
  registerValue<Curve45RailTile>(L, "CURVE_45_RAIL_TILE");
  registerValue<Curve90RailTile>(L, "CURVE_90_RAIL_TILE");
  registerValue<Cross45RailTile>(L, "CROSS_45_RAIL_TILE");
  registerValue<Cross90RailTile>(L, "CROSS_90_RAIL_TILE");
  registerValue<Bridge45RightRailTile>(L, "BRIDGE_45_RIGHT_RAIL_TILE");
  registerValue<Bridge45LeftRailTile>(L, "BRIDGE_45_LEFT_RAIL_TILE");
  registerValue<Bridge90RailTile>(L, "BRIDGE_90_RAIL_TILE");
  registerValue<TurnoutSingleSlipRailTile>(L, "TURNOUT_SINGLE_SLIP_RAIL_TILE");
  registerValue<TurnoutLeft90RailTile>(L, "TURNOUT_LEFT_90_RAIL_TILE");
  registerValue<TurnoutRight45RailTile>(L, "TURNOUT_RIGHT_45_RAIL_TILE");
  registerValue<TurnoutLeft45RailTile>(L, "TURNOUT_LEFT_45_RAIL_TILE");
  registerValue<TurnoutRight90RailTile>(L, "TURNOUT_RIGHT_90_RAIL_TILE");
  registerValue<TurnoutDoubleSlipRailTile>(L, "TURNOUT_DOUBLE_SLIP_RAIL_TILE");
  registerValue<TurnoutWyeRailTile>(L, "TURNOUT_WYE_RAIL_TILE");
  registerValue<TurnoutLeftCurvedRailTile>(L, "TURNOUT__RAIL_TILE");
  registerValue<Turnout3WayRailTile>(L, "TURNOUT_3WAY_RAIL_TILE");
  registerValue<TurnoutRightCurvedRailTile>(L, "TURNOUT_RIGHT_CURVED_RAIL_TILE");
  registerValue<Signal2AspectRailTile>(L, "SIGNAL_2_ASPECT_RAIL_TILE");
  registerValue<Signal3AspectRailTile>(L, "SIGNAL_3_ASPECT_RAIL_TILE");
  registerValue<SensorRailTile>(L, "SENSOR_RAIL_TILE");
  registerValue<BlockRailTile>(L, "BLOCK_RAIL_TILE");

  registerValue<Clock>(L, "CLOCK");

  // hardware - interface:
  registerValue<DCCPlusPlusInterface>(L, "DCCPLUSPLUS_INTERFACE");
  registerValue<ECoSInterface>(L, "ECOS_INTERFACE");
  registerValue<HSI88Interface>(L, "HSI88_INTERFACE");
  registerValue<LocoNetInterface>(L, "LOCONET_INTERFACE");
  registerValue<MarklinCANInterface>(L, "MARKLIN_CAN_INTERFACE");
  registerValue<TraintasticDIYInterface>(L, "TRAINTASTIC_DIY_INTERFACE");
  registerValue<XpressNetInterface>(L, "XPRESSNET_INTERFACE");
  registerValue<WiThrottleInterface>(L, "WITHROTTLE_INTERFACE");
  registerValue<WlanMausInterface>(L, "WLANMAUS_INTERFACE");
  registerValue<Z21Interface>(L, "Z21_INTERFACE");
  registerValue<InterfaceStatus>(L, "INTERFACE_STATUS");

  registerValue<DecoderFunction>(L, "DECODER_FUNCTION");
  registerValue<DecoderList>(L, "DECODER_LIST");
  registerValue<Decoder>(L, "DECODER");
  registerValue<DecoderFunctions>(L, "DECODER_FUNCTIONS");

  registerValue<Input>(L, "INPUT");
  registerValue<InputList>(L, "INPUT_LIST");

  registerValue<Output>(L, "OUTPUT");
  registerValue<OutputList>(L, "OUTPUT_LIST");

  // hardware - identification:
  registerValue<Identification>(L, "IDENTIFICATION");
  registerValue<IdentificationList>(L, "IDENTIFICATION_LIST");

  registerValue<RailVehicleList>(L, "RAIL_VEHICLE_LIST");
  registerValue<Locomotive>(L, "LOCOMOTIVE");
  registerValue<FreightWagon>(L, "FREIGHT_WAGON");

  registerValue<Train>(L, "TRAIN");
  registerValue<TrainList>(L, "TRAIN_LIST");

  registerValue<World>(L, "WORLD");
}

void Class::push(lua_State* L, std::string_view classId)
{
  lua_getglobal(L, metaTableName);
  assert(lua_istable(L, -1));
  Lua::push(L, classId);
  lua_rawget(L, -2);
  assert(lua_isuserdata(L, -1));
  lua_insert(L, lua_gettop(L) - 1);
  lua_pop(L, 1);
  assert(lua_isuserdata(L, -1));
}

void Class::push(lua_State* L, const ObjectPtr& object)
{
  push(L, object->getClassId());
}

int Class::__tostring(lua_State* L)
{
  Sandbox::getGlobal(L, metaTableName);

  // get the real table, the global is wrapped for write protection:
  lua_getmetatable(L, -1);
  lua_getfield(L, -1, "__index");
  assert(lua_istable(L, -1));

  // loop over table to find value and the return key
  const int idx = lua_gettop(L);
  lua_pushnil(L);
  while(lua_next(L, idx))
  {
    const bool eq = lua_compare(L, 1, -1, LUA_OPEQ);
    lua_pop(L, 1); // pop value
    if(eq)
      return 1;
  }

  lua_pushlstring(L, NULL, 0);
  return 1;
}

int Class::getClass(lua_State* L)
{
  checkArguments(L, 1);
  if(auto object = test<::Object>(L, 1))
    push(L, object);
  else
    lua_pushnil(L);
  return 1;
}

}
