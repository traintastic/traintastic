/**
 * server/test/train/train.cpp
 *
 * This file is part of the traintastic test suite.
 *
 * Copyright (C) 2023-2024 Reinder Feenstra
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
#include "../../src/core/objectproperty.tpp"
#include "../../src/core/method.tpp"
#include "../../src/world/world.hpp"
#include "../../src/vehicle/rail/railvehiclelist.hpp"
#include "../../src/vehicle/rail/locomotive.hpp"
#include "../../src/train/trainlist.hpp"
#include "../../src/train/train.hpp"
#include "../../src/train/trainvehiclelist.hpp"
#include "../../src/hardware/decoder/decoder.hpp"
#include "../../src/log/logmessageexception.hpp"

TEST_CASE("Activate empty train", "[train]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(worldWeak.lock()->trains->length == 0);

  std::weak_ptr<Train> trainWeak = world->trains->create();
  REQUIRE_FALSE(trainWeak.expired());
  REQUIRE(worldWeak.lock()->trains->length == 1);

  REQUIRE_FALSE(trainWeak.lock()->active.value());
  CHECK_THROWS_AS(trainWeak.lock()->active = true, invalid_value_error);
  REQUIRE_FALSE(trainWeak.lock()->active.value());
}

TEST_CASE("Delete active train", "[train]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(world->trains->length == 0);

  std::weak_ptr<RailVehicle> locomotiveWeak = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak.expired());
  REQUIRE(world->railVehicles->length == 1);

  std::weak_ptr<Train> trainWeak = world->trains->create();
  REQUIRE_FALSE(trainWeak.expired());
  REQUIRE(world->trains->length == 1);

  REQUIRE(trainWeak.lock()->vehicles->length == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  REQUIRE(trainWeak.lock()->vehicles->length == 1);

  REQUIRE_FALSE(trainWeak.lock()->active.value());
  trainWeak.lock()->active = true;
  REQUIRE(trainWeak.lock()->active.value());

  CHECK_THROWS_AS(world->trains->delete_(trainWeak.lock()), LogMessageException);
  REQUIRE(world->trains->length == 1);

  world.reset();
  REQUIRE(locomotiveWeak.expired());
  REQUIRE(trainWeak.expired());
  REQUIRE(worldWeak.expired());
}

TEST_CASE("Delete inactive train", "[train]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(world->trains->length == 0);

  std::weak_ptr<RailVehicle> locomotiveWeak = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak.expired());
  REQUIRE(world->railVehicles->length == 1);

  std::weak_ptr<Train> trainWeak = world->trains->create();
  REQUIRE_FALSE(trainWeak.expired());
  REQUIRE(world->trains->length == 1);

  REQUIRE(trainWeak.lock()->vehicles->length == 0);
  REQUIRE(locomotiveWeak.lock()->trains.size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  REQUIRE(trainWeak.lock()->vehicles->length == 1);
  REQUIRE(locomotiveWeak.lock()->trains.size() == 1);

  CHECK_NOTHROW(world->trains->delete_(trainWeak.lock()));
  REQUIRE(world->trains->length == 0);
  REQUIRE(trainWeak.expired());
  REQUIRE(locomotiveWeak.lock()->trains.size() == 0);

  world.reset();
  REQUIRE(locomotiveWeak.expired());
  REQUIRE(worldWeak.expired());
}

TEST_CASE("Delete rail vehicle in active train", "[train]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(world->trains->length == 0);

  std::weak_ptr<RailVehicle> locomotiveWeak = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak.expired());
  REQUIRE(world->railVehicles->length == 1);

  std::weak_ptr<Train> trainWeak = world->trains->create();
  REQUIRE_FALSE(trainWeak.expired());
  REQUIRE(world->trains->length == 1);

  REQUIRE(trainWeak.lock()->vehicles->length == 0);
  REQUIRE(locomotiveWeak.lock()->trains.size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  REQUIRE(trainWeak.lock()->vehicles->length == 1);
  REQUIRE(locomotiveWeak.lock()->trains.size() == 1);

  REQUIRE_FALSE(trainWeak.lock()->active.value());
  trainWeak.lock()->active = true;
  REQUIRE(trainWeak.lock()->active.value());

  CHECK_THROWS_AS(world->railVehicles->delete_(locomotiveWeak.lock()), LogMessageException);
  REQUIRE(world->railVehicles->length == 1);

  world.reset();
  REQUIRE(locomotiveWeak.expired());
  REQUIRE(trainWeak.expired());
  REQUIRE(worldWeak.expired());
}

TEST_CASE("Delete rail vehicle in inactive train", "[train]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(world->trains->length == 0);

  std::weak_ptr<RailVehicle> locomotiveWeak = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak.expired());
  REQUIRE(world->railVehicles->length == 1);

  std::weak_ptr<Train> trainWeak = world->trains->create();
  REQUIRE_FALSE(trainWeak.expired());
  REQUIRE(world->trains->length == 1);

  REQUIRE(trainWeak.lock()->vehicles->length == 0);
  REQUIRE(locomotiveWeak.lock()->trains.size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  REQUIRE(trainWeak.lock()->vehicles->length == 1);
  REQUIRE(locomotiveWeak.lock()->trains.size() == 1);

  CHECK_NOTHROW(world->railVehicles->delete_(locomotiveWeak.lock()));
  REQUIRE(world->railVehicles->length == 0);
  REQUIRE(trainWeak.lock()->vehicles->length == 0);

  world.reset();
  REQUIRE(locomotiveWeak.expired());
  REQUIRE(trainWeak.expired());
  REQUIRE(worldWeak.expired());
}
