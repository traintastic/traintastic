#include <catch2/catch.hpp>
#include "../../src/world/world.hpp"

TEST_CASE("Lua script: no code, start/stop, disable", "[lua][lua-script]")
{
  auto world = World::create();
  REQUIRE(world);

  auto script = world->luaScripts->add();
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
