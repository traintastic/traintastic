/**
 * server/test/lua/script/persistentvariables.cpp
 *
 * This file is part of the traintastic test suite.
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

#include <catch2/catch.hpp>
#include "../../../src/lua/scriptlist.hpp"
#include "../../../src/train/train.hpp"
#include "../../../src/train/trainlist.hpp"
#include "../../../src/utils/endswith.hpp"
#include "../../../src/world/world.hpp"

TEST_CASE("Lua script: pv - save/restore - bool", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = true";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv.test == true)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - int", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = 42";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv.test == 42)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - float", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = math.pi";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv.test == math.pi)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - string", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = 'traintastic'";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv.test == 'traintastic')";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - enum", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = enum.world_event.RUN";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv.test == enum.world_event.RUN)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - set", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = set.world_state.RUN";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv.test == set.world_state.RUN)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - object", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = world";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv.test == world)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - vector property", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);
  auto train = world->trains->create();
  REQUIRE(train);
  train->id = "train";

  // set pv:
  script->code = "pv.test = world.get_object('train').blocks";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv.test == world.get_object('train').blocks)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
  train.reset();
}

TEST_CASE("Lua script: pv - save/restore - method", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = world.stop";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv.test == world.stop)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - event", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = world.on_event";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv.test == world.on_event)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - unsupported - function", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = function(a, b)\nreturn a+b\nend";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Error);
  REQUIRE(endsWith(script->error.value(), "can't store value as persistent variable, unsupported type"));

  script.reset();
}

TEST_CASE("Lua script: pv - unsupported - table", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = {}";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Error);
  REQUIRE(endsWith(script->error.value(), "can't store value as persistent variable, unsupported type"));

  script.reset();
}
