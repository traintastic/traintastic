/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#include "scriptthrottle.hpp"
#include "object.hpp"
#include "../check.hpp"
#include "../checkarguments.hpp"
#include "../push.hpp"
#include "../to.hpp"
#include "../metatable.hpp"
#include "../../throttle/scriptthrottle.hpp"
#include "../../train/train.hpp"

namespace Lua::Object {

void ScriptThrottle::registerType(lua_State* L)
{
  MetaTable::clone(L, Object::metaTableName, metaTableName);
  lua_pushcfunction(L, __index);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);
}

int ScriptThrottle::index(lua_State* L, ::ScriptThrottle& object)
{
  const auto key = to<std::string_view>(L, 2);
  LUA_OBJECT_METHOD(acquire)
  LUA_OBJECT_METHOD(release)
  return Object::index(L, object);
}

int ScriptThrottle::__index(lua_State* L)
{
  return index(L, *check<::ScriptThrottle>(L, 1));
}

int ScriptThrottle::acquire(lua_State* L)
{
  const int n = checkArguments(L, 1, 2);
  auto throttle = check<::ScriptThrottle>(L, lua_upvalueindex(1));

  const auto ec = throttle->acquire(check<Train>(L, 1), (n >= 2) && to<bool>(L, 2));
  Lua::push(L, ec.value());
  return 1;
}

int ScriptThrottle::release(lua_State* L)
{
  const int n = checkArguments(L, 0, 1);
  check<::ScriptThrottle>(L, lua_upvalueindex(1))->release((n < 1) || to<bool>(L, 1));
  return 0;
}

}
