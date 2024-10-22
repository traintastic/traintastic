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
#include "../../../src/core/method.tpp"
#include "../../../src/core/objectproperty.tpp"
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

TEST_CASE("Lua script: pv - save/restore - table, empty", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = {}";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv.test ~= nil)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - table, array like", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = {1, 2, 3}";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv.test[1] == 1)\nassert(pv.test[2] == 2)\nassert(pv.test[3] == 3)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - table, array length", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = {1, 2, 3}";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(#pv.test == 3)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - table, map like", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv.test = {one=1, two=2}";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv.test.one == 1)\nassert(pv.test.two == 2)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - bool as key", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv[true] = false";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv[true] == false)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - float as key", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv[math.pi] = 3";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv[math.pi] == 3)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - enum key as key", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv[enum.world_event.RUN] = 'y'";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv[enum.world_event.RUN] == 'y')";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - set as key", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv[set.world_state.RUN] = 'test'";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv[set.world_state.RUN] == 'test')";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - object as key", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv[world] = 'world'";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv[world] == 'world')";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - vector property as key", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);
  auto train = world->trains->create();
  REQUIRE(train);
  train->id = "train";

  // set pv:
  script->code = "pv[world.get_object('train').blocks] = 'test'";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv[world.get_object('train').blocks] == 'test')";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
  train.reset();
}

TEST_CASE("Lua script: pv - save/restore - method as key", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv[world.stop] = 'test'";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv[world.stop] == 'test')";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - save/restore - event as key", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv[world.on_event] = 'test'";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code = "assert(pv[world.on_event] == 'test')";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - unsupported - function as key", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv[function(a, b)\nreturn a+b\nend] = 'test'";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Error);
  REQUIRE(endsWith(script->error.value(), "can't store value as persistent variable, unsupported type"));

  script.reset();
}

TEST_CASE("Lua script: pv - unsupported - table as key", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code = "pv[{}] = 'test'";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Error);
  REQUIRE(endsWith(script->error.value(), "can't store value as persistent variable, unsupported type"));

  script.reset();
}

TEST_CASE("Lua script: pv - unsupported - table recursion", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code =
    "t = {}\n"
    "t['t'] = t\n"
    "pv['t'] = t";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Error);
  REQUIRE(endsWith(script->error.value(), "table contains recursion"));

  script.reset();
}

TEST_CASE("Lua script: pv - unsupported - table recursion 2", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code =
    "t = {}\n"
    "a = {}\n"
    "b = {}\n"
    "t['a'] = a\n"
    "t['b'] = b\n"
    "t['a']['b'] = b\n"
    "t['b']['a'] = a\n"
    "pv['t'] = t";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Error);
  REQUIRE(endsWith(script->error.value(), "table contains recursion"));

  script.reset();
}

TEST_CASE("Lua script: pv - pairs()", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code =
    "pv.test = {a=1, b=2, c=3}\n"
    "for k, v in pairs(pv.test) do\n"
    "  assert((k == 'a' and v == 1) or (k == 'b' and v == 2) or (k == 'c' and v == 3))\n"
    "end";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code =
    "for k, v in pairs(pv.test) do\n"
    "  assert((k == 'a' and v == 1) or (k == 'b' and v == 2) or (k == 'c' and v == 3))\n"
    "end";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}

TEST_CASE("Lua script: pv - ipairs()", "[lua][lua-script][lua-script-pv]")
{
  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // set pv:
  script->code =
    "pv.test = {5, 4, 3, 2, 1}\n"
    "assert(#pv.test == 5)\n"
    "local n = 1\n"
    "for k, v in ipairs(pv.test) do\n"
    "  assert(k == n)\n"
    "  assert(v == (#pv.test - n + 1))\n"
    "  n = n + 1\n"
    "end";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  // check pv:
  script->code =
    "assert(#pv.test == 5)\n"
    "local n = 1\n"
    "for k, v in ipairs(pv.test) do\n"
    "  assert(k == n)\n"
    "  assert(v == (#pv.test - n + 1))\n"
    "  n = n + 1\n"
    "end";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}
