/**
 * server/test/lua/script/deadobject.cpp
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

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "../../../src/core/eventloop.hpp"
#include "../../../src/core/method.tpp"
#include "../../../src/core/objectproperty.tpp"
#include "../../../src/log/log.hpp"
#include "../../../src/log/memorylogger.hpp"
#include "../../../src/lua/scriptlist.hpp"
#include "../../../src/train/train.hpp"
#include "../../../src/train/trainlist.hpp"
#include "../../../src/utils/endswith.hpp"
#include "../../../src/world/world.hpp"

using Catch::Matchers::EndsWith;

TEST_CASE("Lua script: dead object", "[lua][lua-script][lua-script-dead-object]")
{
  Log::enableMemoryLogger(100);
  EventLoop::threadId = std::this_thread::get_id(); // else MemoryLogger will post it to the event loop

  auto world = World::create();
  REQUIRE(world);
  auto script = world->luaScripts->create();
  REQUIRE(script);

  // create train:
  auto train = world->trains->create();
  REQUIRE(train);
  std::weak_ptr<Train> trainWeak = train;
  train->id = "train";
  train->name = "Train";
  train.reset();

  // read name:
  script->code =
    "local train = world.get_object(\"train\")\n"
    "assert(train.name == \"Train\")\n"
    "world.on_event(\n"
    "  function ()\n"
    "    assert(train.name == \"Train\")\n"
    "  end)";
  script->start();
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running);

  // delete train:
  world->trains->delete_(trainWeak.lock());
  REQUIRE(trainWeak.expired());

  // trigger event to call dead object:
  world->simulation = true;
  INFO(script->error.value());
  REQUIRE(script->state.value() == LuaScriptState::Running); // error in event does not stop script

  // check log:
  REQUIRE(Log::getMemoryLogger());
  auto& logger = *Log::getMemoryLogger();
  REQUIRE(logger.size() != 0);
  auto& lastLog = logger[logger.size() - 1];

  REQUIRE(lastLog.message == LogMessage::E9001_X_DURING_EXECUTION_OF_X_EVENT_HANDLER);
  REQUIRE(lastLog.args);
  auto args = *lastLog.args;
  REQUIRE(args.size() == 2);
  REQUIRE_THAT(args[0], EndsWith("dead object"));
  REQUIRE(args[1] == "world.on_event");

  // stop script:
  script->stop();
  REQUIRE(script->state.value() == LuaScriptState::Stopped);

  script.reset();
}
