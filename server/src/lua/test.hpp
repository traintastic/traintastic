/**
 * server/src/lua/test.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LUA_TEST_HPP
#define TRAINTASTIC_SERVER_LUA_TEST_HPP

#include <lua.hpp>
#include <type_traits>
#include "error.hpp"
#include "metatable.hpp"
#include "../core/object.hpp"
#include "../utils/startswith.hpp"

namespace Lua {

template<typename T>
std::shared_ptr<T> test(lua_State* L, int index)
{
  if constexpr(std::is_same_v<T, ::Object> || std::is_base_of_v<::Object, T>)
  {
    auto name = MetaTable::getName(L, index);
    if(name == "object" || startsWith(name, "object."))
    {
      auto object = static_cast<ObjectPtrWeak*>(lua_touserdata(L, index))->lock();

      if(!object)
        errorDeadObject(L);

      if constexpr(std::is_same_v<T, ::Object>)
        return object;

      if(auto objectT = std::dynamic_pointer_cast<T>(object))
        return objectT;
    }
    return {};
  }
  else
    static_assert(sizeof(T) != sizeof(T), "don't know how to test type");
}

}

#endif
