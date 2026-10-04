/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include <catch2/catch_test_macros.hpp>
#include "../../../src/core/eventloop.hpp"
#include "../../../src/core/method.tpp"
#include "../../../src/core/objectproperty.tpp"
#include "../../../src/log/log.hpp"
#include "../../../src/log/memorylogger.hpp"
#include "../../../src/lua/scriptlist.hpp"
#include "../../../src/world/world.hpp"

namespace Lua {

struct ScriptTestAccess
{
  static nlohmann::json getPersistentVariables(const std::weak_ptr<Lua::Script>& script)
  {
    auto pv = nlohmann::json::object();
    for(const auto& item : script.lock()->m_persistentVariables["items"])
    {
      pv.emplace(item["key"].get<std::string_view>(), item["value"]);
    }
    return pv;
  }
};

}

TEST_CASE("Lua script: object.on_changed - all", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code =
    "pv.count = 0\n"
    "pv.name = 0\n"
    "pv.scale = 0\n"
    "pv.scale_ratio = 0\n"
    "world.on_changed(\n"
    "  function (value, object, name, user_data)\n"
    "    pv.count = pv.count + 1\n"
    "    if name == \"name\" then\n"
    "      pv.name = pv.name + 1\n"
    "      assert(value == \"Traintastic!\")\n"
    "    elseif name == \"scale\" then\n"
    "      pv.scale = pv.scale + 1\n"
    "      assert(value == enum.world_scale.TT)\n"
    "    elseif name == \"scale_ratio\" then\n"
    "      pv.scale_ratio = pv.scale_ratio + 1\n"
    "      assert(value == 120)\n"
    "    else\n"
    "      assert(false)\n"
    "    end\n"
    "    assert(object == world)\n"
    "    assert(user_data == 42)\n"
    "  end, 42)";
  scriptWeak.lock()->start();
  INFO(scriptWeak.lock()->error.value());
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Running);

  // trigger on_changed:
  world->name = "Traintastic!";
  world->scale = WorldScale::TT;

  // check log:
  REQUIRE(Log::getMemoryLogger());
  auto& logger = *Log::getMemoryLogger();
  REQUIRE(logger.size() != 0);
  auto& lastLog = logger[logger.size() - 1];
  REQUIRE(lastLog.message == LogMessage::N9001_STARTING_SCRIPT);

  // stop script to be able to read PV:
  scriptWeak.lock()->stop();
  INFO(scriptWeak.lock()->error.value());
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Stopped);

  const auto pv = Lua::ScriptTestAccess::getPersistentVariables(scriptWeak);
  INFO(pv.dump());
  REQUIRE(pv["count"] == 3);
  REQUIRE(pv["name"] == 1);
  REQUIRE(pv["scale"] == 1);
  REQUIRE(pv["scale_ratio"] == 1);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - name only - string", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code =
    "pv.count = 0\n"
    "pv.name = 0\n"
    "world.on_changed(\"name\", \n"
    "  function (value, object, name, user_data)\n"
    "    pv.count = pv.count + 1\n"
    "    if name == \"name\" then\n"
    "      pv.name = pv.name + 1\n"
    "      assert(value == \"Traintastic!\")\n"
    "    else\n"
    "      assert(false)\n"
    "    end\n"
    "    assert(object == world)\n"
    "    assert(user_data == 42)\n"
    "  end, 42)";
  scriptWeak.lock()->start();
  INFO(scriptWeak.lock()->error.value());
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Running);

  // trigger on_changed:
  world->name = "Traintastic!";
  world->scale = WorldScale::TT; // also modifies scaleRatio

  // check log:
  REQUIRE(Log::getMemoryLogger());
  auto& logger = *Log::getMemoryLogger();
  REQUIRE(logger.size() != 0);
  auto& lastLog = logger[logger.size() - 1];
  REQUIRE(lastLog.message == LogMessage::N9001_STARTING_SCRIPT);

  // stop script to be able to read PV:
  scriptWeak.lock()->stop();
  INFO(scriptWeak.lock()->error.value());
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Stopped);

  const auto pv = Lua::ScriptTestAccess::getPersistentVariables(scriptWeak);
  INFO(pv.dump());
  REQUIRE(pv["count"] == 1);
  REQUIRE(pv["name"] == 1);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - name only - table", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code =
    "pv.count = 0\n"
    "pv.name = 0\n"
    "world.on_changed({\"name\"}, \n"
    "  function (value, object, name, user_data)\n"
    "    pv.count = pv.count + 1\n"
    "    if name == \"name\" then\n"
    "      pv.name = pv.name + 1\n"
    "      assert(value == \"Traintastic!\")\n"
    "    else\n"
    "      assert(false)\n"
    "    end\n"
    "    assert(object == world)\n"
    "    assert(user_data == 42)\n"
    "  end, 42)";
  scriptWeak.lock()->start();
  INFO(scriptWeak.lock()->error.value());
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Running);

  // trigger on_changed:
  world->name = "Traintastic!";
  world->scale = WorldScale::TT; // also modifies scaleRatio

  // check log:
  REQUIRE(Log::getMemoryLogger());
  auto& logger = *Log::getMemoryLogger();
  REQUIRE(logger.size() != 0);
  auto& lastLog = logger[logger.size() - 1];
  REQUIRE(lastLog.message == LogMessage::N9001_STARTING_SCRIPT);

  // stop script to be able to read PV:
  scriptWeak.lock()->stop();
  INFO(scriptWeak.lock()->error.value());
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Stopped);

  const auto pv = Lua::ScriptTestAccess::getPersistentVariables(scriptWeak);
  INFO(pv.dump());
  REQUIRE(pv["count"] == 1);
  REQUIRE(pv["name"] == 1);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - name and scale_ratio", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code =
    "pv.count = 0\n"
    "pv.name = 0\n"
    "pv.scale_ratio = 0\n"
    "world.on_changed({\"name\", \"scale_ratio\"}, \n"
    "  function (value, object, name, user_data)\n"
    "    pv.count = pv.count + 1\n"
    "    if name == \"name\" then\n"
    "      pv.name = pv.name + 1\n"
    "      assert(value == \"Traintastic!\")\n"
    "    elseif name == \"scale_ratio\" then\n"
    "      pv.scale_ratio = pv.scale_ratio + 1\n"
    "      assert(value == 120)\n"
    "    else\n"
    "      assert(false)\n"
    "    end\n"
    "    assert(object == world)\n"
    "    assert(user_data == 42)\n"
    "  end, 42)";
  scriptWeak.lock()->start();
  INFO(scriptWeak.lock()->error.value());
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Running);

  // trigger on_changed:
  world->name = "Traintastic!";
  world->scale = WorldScale::TT;

  // check log:
  REQUIRE(Log::getMemoryLogger());
  auto& logger = *Log::getMemoryLogger();
  REQUIRE(logger.size() != 0);
  auto& lastLog = logger[logger.size() - 1];
  REQUIRE(lastLog.message == LogMessage::N9001_STARTING_SCRIPT);

  // stop script to be able to read PV:
  scriptWeak.lock()->stop();
  INFO(scriptWeak.lock()->error.value());
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Stopped);

  const auto pv = Lua::ScriptTestAccess::getPersistentVariables(scriptWeak);
  INFO(pv.dump());
  REQUIRE(pv["count"] == 2);
  REQUIRE(pv["name"] == 1);
  REQUIRE(pv["scale_ratio"] == 1);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - invalid arguments - empty table as filter", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code = "world.on_changed({}, function () end)\n";
  scriptWeak.lock()->start();
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Error);
  REQUIRE(scriptWeak.lock()->error.value() == "property list is empty");

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - invalid arguments - non existing property - string", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code = "world.on_changed(\"non_existing_property\", function () end)\n";
  scriptWeak.lock()->start();
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Error);
  REQUIRE(scriptWeak.lock()->error.value() == "unknown property: non_existing_property");

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - invalid arguments - non existing property - table", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code = "world.on_changed({\"name\", \"non_existing_property\"}, function () end)\n";
  scriptWeak.lock()->start();
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Error);
  REQUIRE(scriptWeak.lock()->error.value() == "unknown property: non_existing_property");

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - invalid arguments - nil as filter", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code = "world.on_changed(nil, function () end)\n";
  scriptWeak.lock()->start();
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Error);
  REQUIRE(scriptWeak.lock()->error.value() == "single property name or non empty table with names required");

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - invalid arguments - bool as filter", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code = "world.on_changed(true, function () end)\n";
  scriptWeak.lock()->start();
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Error);
  REQUIRE(scriptWeak.lock()->error.value() == "single property name or non empty table with names required");

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - invalid arguments - number as filter", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code = "world.on_changed(42, function () end)\n";
  scriptWeak.lock()->start();
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Error);
  REQUIRE(scriptWeak.lock()->error.value() == "single property name or non empty table with names required");

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - invalid arguments - object as filter", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code = "world.on_changed(world, function () end)\n";
  scriptWeak.lock()->start();
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Error);
  REQUIRE(scriptWeak.lock()->error.value() == "single property name or non empty table with names required");

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - invalid arguments - zero arguments", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code = "world.on_changed()\n";
  scriptWeak.lock()->start();
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Error);
  REQUIRE(scriptWeak.lock()->error.value() == ":1: expected 1..3 arguments, got 0");

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - invalid arguments - to many arguments", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code = "world.on_changed(function () end, 42, 42)\n";
  scriptWeak.lock()->start();
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Error);
  REQUIRE(scriptWeak.lock()->error.value() == ":1: expected 1..2 arguments, got 3");

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - invalid arguments - missing function argument", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code = "world.on_changed(\"name\")\n";
  scriptWeak.lock()->start();
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Error);
  REQUIRE(scriptWeak.lock()->error.value() == ":1: expected 2..3 arguments, got 1");

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}

TEST_CASE("Lua script: object.on_changed - invalid arguments - non function argument", "[lua][lua-script]")
{
  Log::enableMemoryLogger(100);
  EventLoop::reset();
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Lua::Script> scriptWeak = world->luaScripts->create();
  REQUIRE_FALSE(scriptWeak.expired());

  scriptWeak.lock()->code = "world.on_changed(\"name\", 42)\n";
  scriptWeak.lock()->start();
  REQUIRE(scriptWeak.lock()->state.value() == LuaScriptState::Error);
  REQUIRE(scriptWeak.lock()->error.value() == ":1: bad argument #2 to 'on_changed' (function expected, got number)");

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(scriptWeak.expired());
}
