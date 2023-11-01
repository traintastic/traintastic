/**
 * server/src/lua/enums.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LUA_ENUMS_HPP
#define TRAINTASTIC_SERVER_LUA_ENUMS_HPP

#include <lua.hpp>
#include "enum.hpp"
#include <traintastic/enum/blocktraindirection.hpp>
#include <traintastic/enum/decoderprotocol.hpp>
#include "../../src/enum/direction.hpp"
#include "../../src/enum/directioncontrolstate.hpp"
#include <traintastic/enum/identificationeventtype.hpp>
#include <traintastic/enum/interfacestate.hpp>
#include "../../src/enum/tristate.hpp"
#include "../../src/enum/turnoutposition.hpp"
#include "../../src/enum/signalaspect.hpp"
#include "../../src/enum/worldevent.hpp"
#include "../../src/enum/worldscale.hpp"

#define LUA_ENUMS \
  BlockTrainDirection, \
  DecoderProtocol, \
  Direction, \
  DirectionControlState, \
  IdentificationEventType, \
  InterfaceState, \
  TriState, \
  TurnoutPosition, \
  SignalAspect, \
  WorldEvent, \
  WorldScale

namespace Lua {

struct Enums
{
  template<class T, class... Ts>
  inline static void registerTypes(lua_State* L)
  {
    Enum<T>::registerType(L);
    if constexpr(sizeof...(Ts) != 0)
      registerTypes<Ts...>(L);
  }

  template<class T, class... Ts>
  inline static void registerValues(lua_State* L)
  {
    Enum<T>::registerValues(L);
    if constexpr(sizeof...(Ts) != 0)
      registerValues<Ts...>(L);
  }
};

}

#endif

