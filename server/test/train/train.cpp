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
#include "../../src/core/eventloop.hpp"
#include "../../src/core/objectproperty.tpp"
#include "../../src/core/method.tpp"
#include "../../src/world/world.hpp"
#include "../../src/vehicle/rail/railvehiclelist.hpp"
#include "../../src/vehicle/rail/locomotive.hpp"
#include "../../src/train/trainlist.hpp"
#include "../../src/train/train.hpp"
#include "../../src/train/trainvehiclelist.hpp"
#include "../../src/train/trainvehiclelistitem.hpp"
#include "../../src/hardware/decoder/decoder.hpp"
#include "../../src/log/logmessageexception.hpp"

TEST_CASE("Activate empty train", "[train]")
{
  EventLoop::reset();

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
  EventLoop::reset();

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

  REQUIRE(trainWeak.lock()->vehicles->size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  REQUIRE(trainWeak.lock()->vehicles->size() == 1);

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
  EventLoop::reset();

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

  REQUIRE(trainWeak.lock()->vehicles->size() == 0);
  REQUIRE(locomotiveWeak.lock()->trains.size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  REQUIRE(trainWeak.lock()->vehicles->size() == 1);
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
  EventLoop::reset();

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

  REQUIRE(trainWeak.lock()->vehicles->size() == 0);
  REQUIRE(locomotiveWeak.lock()->trains.size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  REQUIRE(trainWeak.lock()->vehicles->size() == 1);
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
  EventLoop::reset();

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

  REQUIRE(trainWeak.lock()->vehicles->size() == 0);
  REQUIRE(locomotiveWeak.lock()->trains.size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  REQUIRE(trainWeak.lock()->vehicles->size() == 1);
  REQUIRE(locomotiveWeak.lock()->trains.size() == 1);

  CHECK_NOTHROW(world->railVehicles->delete_(locomotiveWeak.lock()));
  REQUIRE(world->railVehicles->length == 0);
  REQUIRE(trainWeak.lock()->vehicles->size() == 0);

  world.reset();
  REQUIRE(locomotiveWeak.expired());
  REQUIRE(trainWeak.expired());
  REQUIRE(worldWeak.expired());
}

TEST_CASE("Check direction propagation", "[train]")
{
  EventLoop::reset();

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(world->trains->length == 0);

  std::weak_ptr<RailVehicle> locomotiveWeak1 = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak1.expired());
  REQUIRE(world->railVehicles->length == 1);
  locomotiveWeak1.lock()->decoder->direction = Direction::Forward;

  std::weak_ptr<RailVehicle> locomotiveWeak2 = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak2.expired());
  REQUIRE(world->railVehicles->length == 2);
  locomotiveWeak2.lock()->decoder->direction = Direction::Reverse;

  std::weak_ptr<Train> trainWeak = world->trains->create();
  REQUIRE_FALSE(trainWeak.expired());
  REQUIRE(world->trains->length == 1);

  REQUIRE(trainWeak.lock()->vehicles->size() == 0);
  REQUIRE(locomotiveWeak1.lock()->trains.size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak1.lock());
  REQUIRE(trainWeak.lock()->vehicles->size() == 1);
  REQUIRE(locomotiveWeak1.lock()->trains.size() == 1);

  REQUIRE(locomotiveWeak2.lock()->trains.size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak2.lock());
  REQUIRE(trainWeak.lock()->vehicles->size() == 2);
  REQUIRE(locomotiveWeak2.lock()->trains.size() == 1);

  // Inactive train does not propagate direction
  REQUIRE(trainWeak.lock()->active == false);
  trainWeak.lock()->direction = Direction::Forward;
  REQUIRE(locomotiveWeak1.lock()->decoder->direction == Direction::Forward);
  REQUIRE(locomotiveWeak2.lock()->decoder->direction == Direction::Reverse);

  trainWeak.lock()->direction = Direction::Reverse;
  REQUIRE(locomotiveWeak1.lock()->decoder->direction == Direction::Forward);
  REQUIRE(locomotiveWeak2.lock()->decoder->direction == Direction::Reverse);

  // Now activate train and check direction was propagated
  trainWeak.lock()->active = true;
  REQUIRE(trainWeak.lock()->active == true);
  REQUIRE(locomotiveWeak1.lock()->activeTrain.value() == trainWeak.lock());
  REQUIRE(locomotiveWeak2.lock()->activeTrain.value() == trainWeak.lock());
  REQUIRE(trainWeak.lock()->direction == Direction::Reverse);
  REQUIRE(locomotiveWeak1.lock()->decoder->direction == Direction::Reverse);
  REQUIRE(locomotiveWeak2.lock()->decoder->direction == Direction::Reverse);

  trainWeak.lock()->direction = Direction::Forward;
  REQUIRE(locomotiveWeak1.lock()->decoder->direction == Direction::Forward);
  REQUIRE(locomotiveWeak2.lock()->decoder->direction == Direction::Forward);

  // Now invert direction of locomotive 2
  TrainVehicleList& vehicleList = *trainWeak.lock()->vehicles.value(); //TODO: is it the right way to get it?
  vehicleList[1]->invertDirection = true;
  REQUIRE(locomotiveWeak1.lock()->decoder->direction == Direction::Forward);
  REQUIRE(locomotiveWeak2.lock()->decoder->direction == Direction::Reverse);

  // Now invert direction from decoder
  locomotiveWeak1.lock()->decoder->direction = Direction::Reverse;
  REQUIRE(trainWeak.lock()->direction == Direction::Reverse);
  REQUIRE(locomotiveWeak2.lock()->decoder->direction == Direction::Forward);

  // Add new vehicle to active train, its direction will be synced to Train direction
  std::weak_ptr<RailVehicle> locomotiveWeak3 = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak3.expired());
  REQUIRE(world->railVehicles->length == 3);
  locomotiveWeak3.lock()->decoder->direction = Direction::Forward;
  REQUIRE(locomotiveWeak3.lock()->decoder->direction == Direction::Forward);

  REQUIRE(locomotiveWeak3.lock()->trains.size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak3.lock());
  REQUIRE(trainWeak.lock()->vehicles->size() == 3);
  REQUIRE(locomotiveWeak3.lock()->trains.size() == 1);
  REQUIRE(locomotiveWeak3.lock()->activeTrain.value() == trainWeak.lock());

  REQUIRE(trainWeak.lock()->direction == Direction::Reverse);
  REQUIRE(locomotiveWeak1.lock()->decoder->direction == Direction::Reverse);
  REQUIRE(locomotiveWeak2.lock()->decoder->direction == Direction::Forward);
  REQUIRE(locomotiveWeak3.lock()->decoder->direction == Direction::Reverse);

  // Remove vehicle 1 from train
  trainWeak.lock()->vehicles->remove(trainWeak.lock()->vehicles->getItemFromVehicle(locomotiveWeak1.lock()));
  REQUIRE(trainWeak.lock()->vehicles->size() == 2);
  REQUIRE(locomotiveWeak1.lock()->trains.size() == 0);
  REQUIRE(locomotiveWeak1.lock()->activeTrain.value() == nullptr);

  locomotiveWeak1.lock()->decoder->direction = Direction::Forward;
  REQUIRE(trainWeak.lock()->direction == Direction::Reverse);
  REQUIRE(locomotiveWeak1.lock()->decoder->direction == Direction::Forward); // Not in train
  REQUIRE(locomotiveWeak2.lock()->decoder->direction == Direction::Forward); // Inverted
  REQUIRE(locomotiveWeak3.lock()->decoder->direction == Direction::Reverse);

  trainWeak.lock()->direction = Direction::Forward;
  REQUIRE(locomotiveWeak1.lock()->decoder->direction == Direction::Forward); // Not in train
  REQUIRE(locomotiveWeak2.lock()->decoder->direction == Direction::Reverse); // Inverted
  REQUIRE(locomotiveWeak3.lock()->decoder->direction == Direction::Forward);

  locomotiveWeak2.lock()->decoder->direction = Direction::Forward;
  REQUIRE(trainWeak.lock()->direction == Direction::Reverse);
  REQUIRE(locomotiveWeak1.lock()->decoder->direction == Direction::Forward); // Not in train
  REQUIRE(locomotiveWeak2.lock()->decoder->direction == Direction::Forward); // Inverted
  REQUIRE(locomotiveWeak3.lock()->decoder->direction == Direction::Reverse);

  // Deactivate Train, it should no longer listen to decoder changes
  trainWeak.lock()->active = false;
  REQUIRE(trainWeak.lock()->active == false);
  REQUIRE(locomotiveWeak1.lock()->activeTrain.value() == nullptr);
  REQUIRE(locomotiveWeak2.lock()->activeTrain.value() == nullptr);
  REQUIRE(locomotiveWeak3.lock()->activeTrain.value() == nullptr);

  locomotiveWeak3.lock()->decoder->direction = Direction::Forward;
  REQUIRE(trainWeak.lock()->direction == Direction::Reverse);
  REQUIRE(locomotiveWeak2.lock()->decoder->direction == Direction::Forward);

  trainWeak.lock()->direction = Direction::Forward;
  REQUIRE(locomotiveWeak2.lock()->decoder->direction == Direction::Forward);
  REQUIRE(locomotiveWeak3.lock()->decoder->direction == Direction::Forward);

  trainWeak.lock()->direction = Direction::Reverse;
  REQUIRE(locomotiveWeak2.lock()->decoder->direction == Direction::Forward);
  REQUIRE(locomotiveWeak3.lock()->decoder->direction == Direction::Forward);

  world.reset();
  REQUIRE(locomotiveWeak1.expired());
  REQUIRE(locomotiveWeak2.expired());
  REQUIRE(locomotiveWeak3.expired());
  REQUIRE(trainWeak.expired());
  REQUIRE(worldWeak.expired());
}

TEST_CASE("Check emergency stop propagation", "[train]")
{
  EventLoop::reset();

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(world->trains->length == 0);

  std::weak_ptr<RailVehicle> locomotiveWeak1 = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak1.expired());
  REQUIRE(world->railVehicles->length == 1);
  locomotiveWeak1.lock()->decoder->emergencyStop = false;

  std::weak_ptr<RailVehicle> locomotiveWeak2 = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak2.expired());
  REQUIRE(world->railVehicles->length == 2);
  locomotiveWeak2.lock()->decoder->emergencyStop = true;

  std::weak_ptr<Train> trainWeak = world->trains->create();
  REQUIRE_FALSE(trainWeak.expired());
  REQUIRE(world->trains->length == 1);

  REQUIRE(trainWeak.lock()->vehicles->size() == 0);
  REQUIRE(locomotiveWeak1.lock()->trains.size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak1.lock());
  REQUIRE(trainWeak.lock()->vehicles->size() == 1);
  REQUIRE(locomotiveWeak1.lock()->trains.size() == 1);

  REQUIRE(locomotiveWeak2.lock()->trains.size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak2.lock());
  REQUIRE(trainWeak.lock()->vehicles->size() == 2);
  REQUIRE(locomotiveWeak2.lock()->trains.size() == 1);

  // Inactive train does not propagate emergency stop
  REQUIRE(trainWeak.lock()->active == false);
  trainWeak.lock()->emergencyStop = true;
  REQUIRE(locomotiveWeak1.lock()->decoder->emergencyStop == false);
  REQUIRE(locomotiveWeak2.lock()->decoder->emergencyStop == true);

  // Now activate train and check direction was propagated
  trainWeak.lock()->active = true;
  REQUIRE(trainWeak.lock()->active == true);
  REQUIRE(locomotiveWeak1.lock()->activeTrain.value() == trainWeak.lock());
  REQUIRE(locomotiveWeak2.lock()->activeTrain.value() == trainWeak.lock());

  REQUIRE(trainWeak.lock()->emergencyStop == true);
  REQUIRE(locomotiveWeak1.lock()->decoder->emergencyStop == true);
  REQUIRE(locomotiveWeak2.lock()->decoder->emergencyStop == true);

  trainWeak.lock()->emergencyStop = false;
  REQUIRE(trainWeak.lock()->emergencyStop == false);
  REQUIRE(locomotiveWeak1.lock()->decoder->emergencyStop == false);
  REQUIRE(locomotiveWeak2.lock()->decoder->emergencyStop == false);

  // Now toggle emergency stop from decoder
  locomotiveWeak1.lock()->decoder->emergencyStop = true;
  REQUIRE(trainWeak.lock()->emergencyStop == true);
  REQUIRE(locomotiveWeak2.lock()->decoder->emergencyStop == true);

  // Add new vehicle to active, emergency stopped train
  std::weak_ptr<RailVehicle> locomotiveWeak3 = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak3.expired());
  REQUIRE(world->railVehicles->length == 3);
  locomotiveWeak3.lock()->decoder->emergencyStop = false;

  REQUIRE(locomotiveWeak3.lock()->trains.size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak3.lock());
  REQUIRE(trainWeak.lock()->vehicles->size() == 3);
  REQUIRE(locomotiveWeak3.lock()->trains.size() == 1);
  REQUIRE(locomotiveWeak3.lock()->activeTrain.value() == trainWeak.lock());

  REQUIRE(locomotiveWeak1.lock()->decoder->emergencyStop == true);
  REQUIRE(locomotiveWeak2.lock()->decoder->emergencyStop == true);
  REQUIRE(locomotiveWeak3.lock()->decoder->emergencyStop == true);

  // Reset again emergency stop
  trainWeak.lock()->emergencyStop = false;
  REQUIRE(trainWeak.lock()->emergencyStop == false);
  REQUIRE(locomotiveWeak1.lock()->decoder->emergencyStop == false);
  REQUIRE(locomotiveWeak2.lock()->decoder->emergencyStop == false);
  REQUIRE(locomotiveWeak3.lock()->decoder->emergencyStop == false);

  // Add an emergency stopped vehicle to active train
  std::weak_ptr<RailVehicle> locomotiveWeak4 = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak4.expired());
  REQUIRE(world->railVehicles->length == 4);
  locomotiveWeak4.lock()->decoder->emergencyStop = true;

  REQUIRE(locomotiveWeak4.lock()->trains.size() == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak4.lock());
  REQUIRE(trainWeak.lock()->vehicles->size() == 4);
  REQUIRE(locomotiveWeak4.lock()->trains.size() == 1);
  REQUIRE(locomotiveWeak4.lock()->activeTrain.value() == trainWeak.lock());

  REQUIRE(trainWeak.lock()->emergencyStop == true);
  REQUIRE(locomotiveWeak1.lock()->decoder->emergencyStop == true);
  REQUIRE(locomotiveWeak2.lock()->decoder->emergencyStop == true);
  REQUIRE(locomotiveWeak3.lock()->decoder->emergencyStop == true);
  REQUIRE(locomotiveWeak4.lock()->decoder->emergencyStop == true);

  // Remove vehicle 1 from train
  trainWeak.lock()->vehicles->remove(trainWeak.lock()->vehicles->getItemFromVehicle(locomotiveWeak1.lock()));
  REQUIRE(trainWeak.lock()->vehicles->size() == 3);
  REQUIRE(locomotiveWeak1.lock()->trains.size() == 0);
  REQUIRE(locomotiveWeak1.lock()->activeTrain.value() == nullptr);

  trainWeak.lock()->emergencyStop = false;
  REQUIRE(trainWeak.lock()->emergencyStop == false);
  REQUIRE(locomotiveWeak1.lock()->decoder->emergencyStop == true); // Not in train
  REQUIRE(locomotiveWeak2.lock()->decoder->emergencyStop == false);
  REQUIRE(locomotiveWeak3.lock()->decoder->emergencyStop == false);
  REQUIRE(locomotiveWeak4.lock()->decoder->emergencyStop == false);

  locomotiveWeak2.lock()->decoder->emergencyStop = true;
  REQUIRE(trainWeak.lock()->emergencyStop == true);
  REQUIRE(locomotiveWeak1.lock()->decoder->emergencyStop == true); // Not in train
  REQUIRE(locomotiveWeak2.lock()->decoder->emergencyStop == true);
  REQUIRE(locomotiveWeak3.lock()->decoder->emergencyStop == true);
  REQUIRE(locomotiveWeak4.lock()->decoder->emergencyStop == true);

  locomotiveWeak1.lock()->decoder->emergencyStop = false;
  REQUIRE(trainWeak.lock()->emergencyStop == true);
  REQUIRE(locomotiveWeak1.lock()->decoder->emergencyStop == false); // Not in train
  REQUIRE(locomotiveWeak2.lock()->decoder->emergencyStop == true);
  REQUIRE(locomotiveWeak3.lock()->decoder->emergencyStop == true);
  REQUIRE(locomotiveWeak4.lock()->decoder->emergencyStop == true);

  // Deactivate Train, it should no longer listen to decoder changes
  trainWeak.lock()->active = false;
  REQUIRE(trainWeak.lock()->active == false);
  REQUIRE(locomotiveWeak1.lock()->activeTrain.value() == nullptr);
  REQUIRE(locomotiveWeak2.lock()->activeTrain.value() == nullptr);
  REQUIRE(locomotiveWeak3.lock()->activeTrain.value() == nullptr);
  REQUIRE(locomotiveWeak4.lock()->activeTrain.value() == nullptr);

  locomotiveWeak2.lock()->decoder->emergencyStop = false;
  REQUIRE(trainWeak.lock()->emergencyStop == true);
  REQUIRE(locomotiveWeak3.lock()->decoder->emergencyStop == true);

  trainWeak.lock()->emergencyStop = false;
  REQUIRE(trainWeak.lock()->emergencyStop == false);
  REQUIRE(locomotiveWeak2.lock()->decoder->emergencyStop == false);
  REQUIRE(locomotiveWeak3.lock()->decoder->emergencyStop == true);

  world.reset();
  REQUIRE(locomotiveWeak1.expired());
  REQUIRE(locomotiveWeak2.expired());
  REQUIRE(locomotiveWeak3.expired());
  REQUIRE(locomotiveWeak4.expired());
  REQUIRE(trainWeak.expired());
  REQUIRE(worldWeak.expired());
}
