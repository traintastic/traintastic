/**
 * server/src/lua/object/loconetinterface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023-2024 Reinder Feenstra
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

#include "loconetinterface.hpp"
#include "interface.hpp"
#include "object.hpp"
#include "../check.hpp"
#include "../checkarguments.hpp"
#include "../checkvector.hpp"
#include "../push.hpp"
#include "../to.hpp"
#include "../metatable.hpp"

namespace Lua::Object {

void LocoNetInterface::registerType(lua_State* L)
{
  MetaTable::clone(L, Interface::metaTableName, metaTableName);
  lua_pushcfunction(L, __index);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);
}

int LocoNetInterface::index(lua_State* L, ::LocoNetInterface& object)
{
  const auto key = to<std::string_view>(L, 2);
  LUA_OBJECT_METHOD(send)
  LUA_OBJECT_METHOD(imm_packet)
  return Interface::index(L, object);
}

int LocoNetInterface::__index(lua_State* L)
{
  return index(L, *check<::LocoNetInterface>(L, 1));
}

int LocoNetInterface::send(lua_State* L)
{
  checkArguments(L, 1);
  auto interface = check<::LocoNetInterface>(L, lua_upvalueindex(1));
  auto packet = checkVector<uint8_t>(L, 1);
  Lua::push(L, interface->send(packet));
  return 1;
}

int LocoNetInterface::imm_packet(lua_State* L)
{
  const uint8_t defaultRepeat = 2;
  const int argc = checkArguments(L, 1, 2);
  auto interface = check<::LocoNetInterface>(L, lua_upvalueindex(1));
  auto dccPacket = checkVector<uint8_t>(L, 1);
  auto repeat = (argc >= 2) ? check<uint8_t>(L, 2) : defaultRepeat;
  Lua::push(L, interface->immPacket(dccPacket, repeat));
  return 1;
}

}
