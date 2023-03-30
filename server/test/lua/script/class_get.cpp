/**
 * server/test/lua/script/class_get.cpp
 *
 * This file is part of the traintastic test suite.
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

#include <catch2/catch.hpp>
#include "../../hardware/interfaces.hpp"
#include "../../../src/core/objectproperty.tpp"
#include "../../../src/hardware/interface/interfacelist.hpp"
#include "../../../src/hardware/decoder/list/decoderlist.hpp"
#include "../../../src/hardware/input/list/inputlist.hpp"
#include "../../../src/hardware/output/list/outputlist.hpp"
#include "../../../src/world/world.hpp"
#include "../../../src/lua/enums.hpp"
#include "../../../src/lua/sets.hpp"
#include "../../../src/lua/scriptlist.hpp"
#include "../../../src/utils/toupper.hpp"

TEST_CASE("Lua script: class.get() - nil", "[lua][lua-script][lua-script-class-get]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  script->code = "assert(class.get(nil) == nil)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);
}

TEST_CASE("Lua script: class.get() - false", "[lua][lua-script][lua-script-class-get]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  script->code = "assert(class.get(false) == nil)";
  script->start();
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);
}

TEST_CASE("Lua script: class.get() - true", "[lua][lua-script][lua-script-class-get]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  script->code = "assert(class.get(true) == nil)";
  script->start();
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);
}

TEST_CASE("Lua script: class.get() - integer", "[lua][lua-script][lua-script-class-get]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  script->code = "assert(class.get(42) == nil)";
  script->start();
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);
}

TEST_CASE("Lua script: class.get() - number", "[lua][lua-script][lua-script-class-get]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  script->code = "assert(class.get(3.14) == nil)";
  script->start();
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);
}

TEST_CASE("Lua script: class.get() - string", "[lua][lua-script][lua-script-class-get]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  script->code = "assert(class.get(\"traintastic\") == nil)";
  script->start();
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);
}

TEST_CASE("Lua script: class.get() - table", "[lua][lua-script][lua-script-class-get]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  script->code = "assert(class.get({}) == nil)";
  script->start();
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);
}

TEMPLATE_TEST_CASE("Lua script: class.get()", "[lua][lua-script][lua-script-class-get]", LUA_ENUMS)
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  script->code = std::string("assert(class.get(enum.").append(EnumName<TestType>::value).append(".").append(toUpper(EnumValues<TestType>::value.begin()->second)).append(") == nil)");
  script->start();
  INFO(script->code.value());
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);
}

TEMPLATE_TEST_CASE("Lua script: class.get()", "[lua][lua-script][lua-script-class-get]", LUA_SETS)
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  script->code = std::string("assert(class.get(set.").append(set_name_v<TestType>).append(".").append(toUpper(set_values_v<TestType>.begin()->second)).append(") == nil)");
  script->start();
  INFO(script->code.value());
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);
}

TEMPLATE_TEST_CASE("Lua script: class.get()", "[lua][lua-script][lua-script-class-get]", INTERFACES)
{
  auto world = World::create();
  REQUIRE(world);
  auto interface = world->interfaces->create(TestType::classId);
  REQUIRE(interface);
  interface->id = "if";
  REQUIRE(interface->id.value() == "if");
  auto script = world->luaScripts->create();
  REQUIRE(script);

  script->code = "assert(class.get(world.get_object(\"if\")) ~= nil)";
  script->start();
  INFO(script->code.value());
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);
}
