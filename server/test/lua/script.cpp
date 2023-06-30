/**
 * server/test/lua/script.cpp
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
#include "../../src/world/world.hpp"
#include "../../src/core/method.tpp"
#include "../../src/core/objectproperty.tpp"
#include "../../src/lua/scriptlist.hpp"

TEST_CASE("Lua script: no code, start/stop, disable", "[lua][lua-script]")
{
  auto world = World::create();
  REQUIRE(world);

  auto script = world->luaScripts->create();
  REQUIRE(script);

  REQUIRE_FALSE(script->disabled.value());
  REQUIRE(script->code.value().empty());
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script->start();
  REQUIRE(script->state.value() == LuaScriptState::Running);

  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script->disabled = true;
  REQUIRE(script->state.value() == LuaScriptState::Disabled);

  script->start();
  REQUIRE(script->state.value() == LuaScriptState::Disabled);

  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Disabled);

  script.reset();
  world.reset();
}
