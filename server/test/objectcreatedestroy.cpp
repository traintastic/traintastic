/**
 * server/test/objectcreatedestroy.cpp
 *
 * This file is part of the traintastic test suite.
 *
 * Copyright (C) 2021 Reinder Feenstra
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
#include "../src/world/world.hpp"

//#include "../src/hardware/commandstation/dccplusplusserial.hpp"
//#include "../src/hardware/commandstation/loconetserial.hpp"
//#include "../src/hardware/commandstation/loconettcpbinary.hpp"
//#include "../src/hardware/commandstation/rocoz21.hpp"
#include "../src/hardware/commandstation/virtualcommandstation.hpp"
//#include "../src/hardware/commandstation/xpressnetserial.hpp"

TEST_CASE("Create world => destroy world", "[object-create-destroy]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());

  world.reset();
  REQUIRE(worldWeak.expired());
}

TEMPLATE_TEST_CASE("Create world and command station => destroy world", "[object-create-destroy]"
  //, DCCPlusPlusSerial
  //, LocoNetSerial
  //, LocoNetTCPBinary
  //, RocoZ21
  , VirtualCommandStation
  //, XpressNetSerial
  )
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());

  std::weak_ptr<CommandStation> commandStationWeak = world->commandStations->add(TestType::classId);
  REQUIRE_FALSE(commandStationWeak.expired());

  world.reset();
  REQUIRE(commandStationWeak.expired());
  REQUIRE(worldWeak.expired());
}

TEMPLATE_TEST_CASE("Create world and command station => destroy command station", "[object-create-destroy]"
  //, DCCPlusPlusSerial
  //, LocoNetSerial
  //, LocoNetTCPBinary
  //, RocoZ21
  , VirtualCommandStation
  //, XpressNetSerial
  )
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(worldWeak.lock()->commandStations->length == 0);

  std::weak_ptr<CommandStation> commandStationWeak = world->commandStations->add(TestType::classId);
  REQUIRE_FALSE(commandStationWeak.expired());
  REQUIRE(worldWeak.lock()->commandStations->length == 1);

  world->commandStations->remove(commandStationWeak.lock());
  REQUIRE(commandStationWeak.expired());
  REQUIRE(worldWeak.lock()->commandStations->length == 0);

  world.reset();
  REQUIRE(worldWeak.expired());
}

TEST_CASE("Create world and decoder => destroy world", "[object-create-destroy]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());

  std::weak_ptr<Decoder> decoderWeak = world->decoders->add();
  REQUIRE_FALSE(decoderWeak.expired());

  world.reset();
  REQUIRE(decoderWeak.expired());
  REQUIRE(worldWeak.expired());
}

TEST_CASE("Create world and decoder => destroy decoder", "[object-create-destroy]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(worldWeak.lock()->decoders->length == 0);

  std::weak_ptr<Decoder> decoderWeak = world->decoders->add();
  REQUIRE_FALSE(decoderWeak.expired());
  REQUIRE(worldWeak.lock()->decoders->length == 1);

  world->decoders->remove(decoderWeak.lock());
  REQUIRE(decoderWeak.expired());
  REQUIRE(worldWeak.lock()->decoders->length == 0);

  world.reset();
  REQUIRE(worldWeak.expired());
}

TEMPLATE_TEST_CASE("Create world, command station and decoder => destroy command station", "[object-create-destroy]"
  //, DCCPlusPlusSerial
  //, LocoNetSerial
  //, LocoNetTCPBinary
  //, RocoZ21
  , VirtualCommandStation
  //, XpressNetSerial
  )
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(worldWeak.lock()->commandStations->length == 0);
  REQUIRE(worldWeak.lock()->decoders->length == 0);

  std::weak_ptr<CommandStation> commandStationWeak = world->commandStations->add(TestType::classId);
  REQUIRE_FALSE(commandStationWeak.expired());
  REQUIRE(worldWeak.lock()->commandStations->length == 1);
  REQUIRE(worldWeak.lock()->decoders->length == 0);
  REQUIRE(commandStationWeak.lock()->decoders->length == 0);

  std::weak_ptr<Decoder> decoderWeak = commandStationWeak.lock()->decoders->add();
  REQUIRE_FALSE(decoderWeak.expired());
  REQUIRE(decoderWeak.lock()->commandStation.value() == commandStationWeak.lock());
  REQUIRE(worldWeak.lock()->commandStations->length == 1);
  REQUIRE(worldWeak.lock()->decoders->length == 1);
  REQUIRE(commandStationWeak.lock()->decoders->length == 1);

  world->commandStations->remove(commandStationWeak.lock());
  REQUIRE(commandStationWeak.expired());
  REQUIRE_FALSE(decoderWeak.expired());
  REQUIRE_FALSE(decoderWeak.lock()->commandStation.value().operator bool());
  REQUIRE(worldWeak.lock()->commandStations->length == 0);
  REQUIRE(worldWeak.lock()->decoders->length == 1);

  world.reset();
  REQUIRE(decoderWeak.expired());
  REQUIRE(worldWeak.expired());
}

TEMPLATE_TEST_CASE("Create world, command station and decoder => destroy decoder", "[object-create-destroy]"
  //, DCCPlusPlusSerial
  //, LocoNetSerial
  //, LocoNetTCPBinary
  //, RocoZ21
  , VirtualCommandStation
  //, XpressNetSerial
  )
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(worldWeak.lock()->commandStations->length == 0);
  REQUIRE(worldWeak.lock()->decoders->length == 0);

  std::weak_ptr<CommandStation> commandStationWeak = world->commandStations->add(TestType::classId);
  REQUIRE_FALSE(commandStationWeak.expired());
  REQUIRE(worldWeak.lock()->commandStations->length == 1);
  REQUIRE(worldWeak.lock()->decoders->length == 0);
  REQUIRE(commandStationWeak.lock()->decoders->length == 0);

  std::weak_ptr<Decoder> decoderWeak = commandStationWeak.lock()->decoders->add();
  REQUIRE_FALSE(decoderWeak.expired());
  REQUIRE(decoderWeak.lock()->commandStation.value() == commandStationWeak.lock());
  REQUIRE(worldWeak.lock()->commandStations->length == 1);
  REQUIRE(worldWeak.lock()->decoders->length == 1);
  REQUIRE(commandStationWeak.lock()->decoders->length == 1);

  world->decoders->remove(decoderWeak.lock());
  REQUIRE_FALSE(commandStationWeak.expired());
  REQUIRE(decoderWeak.expired());
  REQUIRE(worldWeak.lock()->commandStations->length == 1);
  REQUIRE(worldWeak.lock()->decoders->length == 0);
  REQUIRE(commandStationWeak.lock()->decoders->length == 0);

  world.reset();
  REQUIRE(commandStationWeak.expired());
  REQUIRE(worldWeak.expired());
}
