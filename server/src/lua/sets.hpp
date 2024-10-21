/**
 * server/src/lua/sets.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LUA_SETS_HPP
#define TRAINTASTIC_SERVER_LUA_SETS_HPP

#include "set.hpp"
#include <array>
#include <string_view>
#include <traintastic/set/worldstate.hpp>

#define LUA_SETS \
  WorldState

namespace Lua {

struct Sets
{
  template<class T, class... Ts>
  inline static void registerTypes(lua_State* L)
  {
    Set<T>::registerType(L);
    if constexpr(sizeof...(Ts) != 0)
      registerTypes<Ts...>(L);
  }

  template<class T, class... Ts>
  inline static void registerValues(lua_State* L)
  {
    Set<T>::registerValues(L);
    if constexpr(sizeof...(Ts) != 0)
      registerValues<Ts...>(L);
  }

  template<class... Ts>
  inline static const std::array<std::string_view, sizeof...(Ts)> getMetaTableNames()
  {
    return std::array<std::string_view, sizeof...(Ts)>{set_name_v<Ts>...};
  }

  inline static const auto metaTableNames = getMetaTableNames<LUA_SETS>();
};

}

#endif

