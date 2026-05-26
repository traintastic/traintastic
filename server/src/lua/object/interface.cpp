/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2024-2026 Reinder Feenstra
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
#include "../enum.hpp"
#include "../push.hpp"
#include "../to.hpp"
#include "../metatable.hpp"
#include "../../hardware/input/inputcontroller.hpp"
#include "../../hardware/input/input.hpp"
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
  if(auto* inputController = dynamic_cast<::InputController*>(&object))
  {
    LUA_OBJECT_PROPERTY(input_channels)
    {
      auto channels = inputController->inputChannels();
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
    LUA_OBJECT_METHOD(get_input)
  }
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

int Interface::get_input(lua_State* L)
{
  checkArguments(L, 1, 3);
  auto inputController = std::dynamic_pointer_cast<::InputController>(check<::Interface>(L, lua_upvalueindex(1)));
  assert(inputController);
  const auto inputChannels = inputController->inputChannels();
  assert(!inputChannels.empty());

  int index = 1;

  InputChannel channel;
  if(inputChannels.size() != 1) // channel required
  {
    channel = Enum<InputChannel>::check(L, index);
    index++;
  }
  else if(Enum<InputChannel>::test(L, index, channel)) // channel optional
  {
    index++;
  }
  else // channel implicit (there is only one)
  {
    channel = inputChannels.front();
  }

  InputLocation location;
  if(hasAddressLocation(channel))
  {
    checkArguments(L, index);
    location = InputAddress(check<uint32_t>(L, index));
  }
  else if(hasNodeAddressLocation(channel))
  {
    checkArguments(L, index + 1);
    location = InputNodeAddress(check<uint32_t>(L, index), check<uint32_t>(L, index + 1));
  }
  else [[unlikely]]
  {
    assert(false);
    errorInternal(L);
  }
  auto& stateData = Lua::Sandbox::getStateData(L);
  auto input = inputController->getInput(channel, location, stateData.script());
  if(input)
  {
    stateData.registerInput(inputController, input);
    Lua::push(L, input);
  }
  else
  {
    Lua::push(L, nullptr);
  }
  return 1;
}

int Interface::get_output(lua_State* L)
{
  checkArguments(L, 2, 3);
  auto outputController = std::dynamic_pointer_cast<::OutputController>(check<::Interface>(L, lua_upvalueindex(1)));
  assert(outputController);
  auto channel = check<::OutputChannel>(L, 1);
  OutputLocation location;
  switch(channel)
  {
    using enum OutputChannel;

    case Output:
    case Accessory:
    case AccessoryDCC:
    case AccessoryMotorola:
    case DCCext:
    case Turnout:
    case ShortEvent:
      checkArguments(L, 2);
      location = OutputAddress(check<uint32_t>(L, 2));
      break;

    case LongEvent:
      checkArguments(L, 3);
      location = OutputNodeAddress(check<uint32_t>(L, 2), check<uint32_t>(L, 3));
      break;

    case ECoSObject:
      checkArguments(L, 2);
      location = OutputECoSObject(check<uint16_t>(L, 2));
      break;
  }
  auto& stateData = Lua::Sandbox::getStateData(L);
  auto output = outputController->getOutput(channel, location, stateData.script());
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
