/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Filippo Gentile
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

#include "xpressnetinterface.hpp"
#include "interface.hpp"
#include "object.hpp"
#include "../check.hpp"
#include "../checkarguments.hpp"
#include "../checkvector.hpp"
#include "../push.hpp"
#include "../to.hpp"
#include "../metatable.hpp"

namespace Lua::Object {

void XpressNetInterface::registerType(lua_State* L)
{
  MetaTable::clone(L, Interface::metaTableName, metaTableName);
  lua_pushcfunction(L, __index);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);
}

int XpressNetInterface::index(lua_State* L, ::XpressNetInterface& object)
{
  const auto key = to<std::string_view>(L, 2);
  LUA_OBJECT_METHOD(send)
  return Interface::index(L, object);
}

int XpressNetInterface::__index(lua_State* L)
{
  return index(L, *check<::XpressNetInterface>(L, 1));
}

int XpressNetInterface::send(lua_State* L)
{
  checkArguments(L, 1);
  auto interface = check<::XpressNetInterface>(L, lua_upvalueindex(1));
  auto message = checkVector<uint8_t>(L, 1);
  Lua::push(L, interface->send(std::move(message)));
  return 1;
}

}
