/**
 * server/src/lua/object/interface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "interface.hpp"
#include <traintastic/enum/outputchannel.hpp>
#include "object.hpp"
#include "../sandbox.hpp"
#include "../script.hpp"
#include "../check.hpp"
#include "../checkarguments.hpp"
#include "../checkvector.hpp"
#include "../push.hpp"
#include "../to.hpp"
#include "../metatable.hpp"
#include "../../hardware/output/outputcontroller.hpp"
#include "../../hardware/output/output.hpp"

namespace Lua::Object {

void Interface::registerType(lua_State* L)
{
  MetaTable::clone(L, Object::metaTableName, metaTableName);
  lua_pushcfunction(L, __index);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);
}

int Interface::index(lua_State* L, ::Interface& object)
{
  const auto key = to<std::string_view>(L, 2);
  if(auto* outputController = dynamic_cast<::OutputController*>(&object))
  {
    LUA_OBJECT_PROPERTY(output_channels)
    {
      auto channels = outputController->outputChannels();
      lua_createtable(L, static_cast<int>(channels.size()), 0);
      lua_Integer n = 1;
      for(auto channel : channels)
      {
        Enum<std::remove_const_t<decltype(channels)::element_type>>::push(L, channel);
        lua_rawseti(L, -2, n);
        n++;
      }
      return 1;
    }
    LUA_OBJECT_METHOD(get_output)
  }
  return Object::index(L, object);
}

int Interface::__index(lua_State* L)
{
  return index(L, *check<::Interface>(L, 1));
}

int Interface::get_output(lua_State* L)
{
  checkArguments(L, 2);
  auto outputController = std::dynamic_pointer_cast<::OutputController>(check<::Interface>(L, lua_upvalueindex(1)));
  assert(outputController);
  auto channel = check<::OutputChannel>(L, 1);
  auto id = check<uint32_t>(L, 2);
  auto& stateData = Lua::Sandbox::getStateData(L);
  auto output = outputController->getOutput(channel, id, stateData.script());
  if(output)
  {
    stateData.registerOutput(outputController, output);
    Lua::push(L, output);
  }
  else
  {
    Lua::push(L, nullptr);
  }
  return 1;
}

}
