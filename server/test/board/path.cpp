/**
 * server/test/board/path.cpp
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
#include "../../src/world/world.hpp"
#include "../../src/core/method.tpp"
#include "../../src/core/objectproperty.tpp"
#include "../../src/board/board.hpp"
#include "../../src/board/boardlist.hpp"
#include "../../src/board/nx/nxmanager.hpp"
#include "../../src/board/tile/rail/blockrailtile.hpp"
#include "../../src/board/tile/rail/bridge90railtile.hpp"
#include "../../src/board/tile/rail/cross90railtile.hpp"
#include "../../src/board/tile/rail/directioncontrolrailtile.hpp"
#include "../../src/board/tile/rail/nxbuttonrailtile.hpp"
#include "../../src/board/tile/rail/turnout/turnoutleft45railtile.hpp"
#include "../../src/board/tile/rail/turnout/turnoutright45railtile.hpp"
#include "../../src/hardware/decoder/decoder.hpp"
#include "../../src/vehicle/rail/railvehiclelist.hpp"
#include "../../src/vehicle/rail/locomotive.hpp"
#include "../../src/train/trainlist.hpp"
#include "../../src/train/train.hpp"
#include "../../src/train/trainvehiclelist.hpp"
#include "../../src/train/trainblockstatus.hpp"

TEST_CASE("Board: Bridge path resevation using NX", "[board][board-path]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  // Board:
  // +--------+                     +--------+
  // | block1 |--(nx1)-\---/-(nx2)--| block2 |
  // +--------+         \ /         +--------+
  //                     X <- bridge
  // +--------+         / \         +--------+
  // | block3 |--(nx3)-/---\-(nx4)--| block4 |
  // +--------+                     +--------+
  std::weak_ptr<Board> boardWeak = world->boards->create();

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 0, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(2, 0, TileRotate::Deg90, TurnoutRight45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(3, 0, TileRotate::Deg90, StraightRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(4, 0, TileRotate::Deg270, TurnoutLeft45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(5, 0, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(6, 0, TileRotate::Deg90, BlockRailTile::classId, false));

  REQUIRE(boardWeak.lock()->addTile(3, 1, TileRotate::Deg45, Bridge90RailTile::classId, false));

  REQUIRE(boardWeak.lock()->addTile(0, 2, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 2, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(2, 2, TileRotate::Deg90, TurnoutLeft45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(3, 2, TileRotate::Deg90, StraightRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(4, 2, TileRotate::Deg270, TurnoutRight45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(5, 2, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(6, 2, TileRotate::Deg90, BlockRailTile::classId, false));

  std::weak_ptr<BlockRailTile> block1 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(block1.expired());
  std::weak_ptr<BlockRailTile> block2 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({6, 0}));
  REQUIRE_FALSE(block2.expired());
  std::weak_ptr<BlockRailTile> block3 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 2}));
  REQUIRE_FALSE(block3.expired());
  std::weak_ptr<BlockRailTile> block4 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({6, 2}));
  REQUIRE_FALSE(block4.expired());

  std::weak_ptr<NXButtonRailTile> nx1 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({1, 0}));
  REQUIRE_FALSE(nx1.expired());
  std::weak_ptr<NXButtonRailTile> nx2 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({5, 0}));
  REQUIRE_FALSE(nx2.expired());
  std::weak_ptr<NXButtonRailTile> nx3 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({1, 2}));
  REQUIRE_FALSE(nx3.expired());
  std::weak_ptr<NXButtonRailTile> nx4 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({5, 2}));
  REQUIRE_FALSE(nx4.expired());

  std::weak_ptr<Bridge90RailTile> bridge = std::dynamic_pointer_cast<Bridge90RailTile>(boardWeak.lock()->getTile({3, 1}));
  REQUIRE_FALSE(bridge.expired());

  // Set blocks free:
  REQUIRE(block2.lock()->state == BlockState::Unknown);
  REQUIRE(block2.lock()->setStateFree());
  REQUIRE(block2.lock()->state == BlockState::Free);
  REQUIRE(block4.lock()->state == BlockState::Unknown);
  REQUIRE(block4.lock()->setStateFree());
  REQUIRE(block4.lock()->state == BlockState::Free);

  // Create two trains:
  std::weak_ptr<RailVehicle> locomotive1 = world->railVehicles->create(Locomotive::classId);
  std::weak_ptr<Train> train1 = world->trains->create();
  REQUIRE(train1.lock()->vehicles->length == 0);
  train1.lock()->vehicles->add(locomotive1.lock());
  REQUIRE(train1.lock()->vehicles->length == 1);

  std::weak_ptr<RailVehicle> locomotive2 = world->railVehicles->create(Locomotive::classId);
  std::weak_ptr<Train> train2 = world->trains->create();
  REQUIRE(train2.lock()->vehicles->length == 0);
  train2.lock()->vehicles->add(locomotive2.lock());
  REQUIRE(train2.lock()->vehicles->length == 1);

  // Assign train 1 to block 1:
  block1.lock()->assignTrain(train1.lock());
  REQUIRE(block1.lock()->state == BlockState::Reserved);
  block1.lock()->flipTrain();

  // Assign train 2 to block 3:
  block3.lock()->assignTrain(train2.lock());
  REQUIRE(block3.lock()->state == BlockState::Reserved);
  block3.lock()->flipTrain();

  // Set world in RUN state (required for selecting paths using NX buttons):
  world->run();

  // Set path for train 1 from block 1 to block 4:
  world->nxManager->select(nx1.lock(), nx4.lock());
  REQUIRE(block4.lock()->state == BlockState::Reserved);
  REQUIRE(block4.lock()->trains.size() == 1);
  REQUIRE(block4.lock()->trains[0]->train.value() == train1.lock());

  // Set path for train 2 from block 3 to block 2:
  world->nxManager->select(nx3.lock(), nx2.lock());
  REQUIRE(block2.lock()->state == BlockState::Reserved);
  REQUIRE(block2.lock()->trains.size() == 1);
  REQUIRE(block2.lock()->trains[0]->train.value() == train2.lock());

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(block1.expired());
  REQUIRE(block2.expired());
  REQUIRE(block3.expired());
  REQUIRE(block4.expired());
  REQUIRE(nx1.expired());
  REQUIRE(nx2.expired());
  REQUIRE(nx3.expired());
  REQUIRE(nx4.expired());
  REQUIRE(bridge.expired());
  REQUIRE(locomotive1.expired());
  REQUIRE(locomotive2.expired());
  REQUIRE(train1.expired());
  REQUIRE(train2.expired());
}

TEST_CASE("Board: Cross path resevation using NX", "[board][board-path]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  // Board:
  // +--------+                     +--------+
  // | block1 |--(nx1)-\---/-(nx2)--| block2 |
  // +--------+         \ /         +--------+
  //                     X <- cross
  // +--------+         / \         +--------+
  // | block3 |--(nx3)-/---\-(nx4)--| block4 |
  // +--------+                     +--------+
  std::weak_ptr<Board> boardWeak = world->boards->create();

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 0, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(2, 0, TileRotate::Deg90, TurnoutRight45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(3, 0, TileRotate::Deg90, StraightRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(4, 0, TileRotate::Deg270, TurnoutLeft45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(5, 0, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(6, 0, TileRotate::Deg90, BlockRailTile::classId, false));

  REQUIRE(boardWeak.lock()->addTile(3, 1, TileRotate::Deg45, Cross90RailTile::classId, false));

  REQUIRE(boardWeak.lock()->addTile(0, 2, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 2, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(2, 2, TileRotate::Deg90, TurnoutLeft45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(3, 2, TileRotate::Deg90, StraightRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(4, 2, TileRotate::Deg270, TurnoutRight45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(5, 2, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(6, 2, TileRotate::Deg90, BlockRailTile::classId, false));

  std::weak_ptr<BlockRailTile> block1 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(block1.expired());
  std::weak_ptr<BlockRailTile> block2 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({6, 0}));
  REQUIRE_FALSE(block2.expired());
  std::weak_ptr<BlockRailTile> block3 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 2}));
  REQUIRE_FALSE(block3.expired());
  std::weak_ptr<BlockRailTile> block4 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({6, 2}));
  REQUIRE_FALSE(block4.expired());

  std::weak_ptr<NXButtonRailTile> nx1 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({1, 0}));
  REQUIRE_FALSE(nx1.expired());
  std::weak_ptr<NXButtonRailTile> nx2 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({5, 0}));
  REQUIRE_FALSE(nx2.expired());
  std::weak_ptr<NXButtonRailTile> nx3 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({1, 2}));
  REQUIRE_FALSE(nx3.expired());
  std::weak_ptr<NXButtonRailTile> nx4 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({5, 2}));
  REQUIRE_FALSE(nx4.expired());

  std::weak_ptr<Cross90RailTile> cross = std::dynamic_pointer_cast<Cross90RailTile>(boardWeak.lock()->getTile({3, 1}));
  REQUIRE_FALSE(cross.expired());

  // Set blocks free:
  REQUIRE(block2.lock()->state == BlockState::Unknown);
  REQUIRE(block2.lock()->setStateFree());
  REQUIRE(block2.lock()->state == BlockState::Free);
  REQUIRE(block4.lock()->state == BlockState::Unknown);
  REQUIRE(block4.lock()->setStateFree());
  REQUIRE(block4.lock()->state == BlockState::Free);

  // Create two trains:
  std::weak_ptr<RailVehicle> locomotive1 = world->railVehicles->create(Locomotive::classId);
  std::weak_ptr<Train> train1 = world->trains->create();
  REQUIRE(train1.lock()->vehicles->length == 0);
  train1.lock()->vehicles->add(locomotive1.lock());
  REQUIRE(train1.lock()->vehicles->length == 1);

  std::weak_ptr<RailVehicle> locomotive2 = world->railVehicles->create(Locomotive::classId);
  std::weak_ptr<Train> train2 = world->trains->create();
  REQUIRE(train2.lock()->vehicles->length == 0);
  train2.lock()->vehicles->add(locomotive2.lock());
  REQUIRE(train2.lock()->vehicles->length == 1);

  // Assign train 1 to block 1:
  block1.lock()->assignTrain(train1.lock());
  REQUIRE(block1.lock()->state == BlockState::Reserved);
  block1.lock()->flipTrain();

  // Assign train 2 to block 3:
  block3.lock()->assignTrain(train2.lock());
  REQUIRE(block3.lock()->state == BlockState::Reserved);
  block3.lock()->flipTrain();

  // Set world in RUN state (required for selecting paths using NX buttons):
  world->run();

  // Set path for train 1 from block 1 to block 4:
  world->nxManager->select(nx1.lock(), nx4.lock());
  REQUIRE(block4.lock()->state == BlockState::Reserved);
  REQUIRE(block4.lock()->trains.size() == 1);
  REQUIRE(block4.lock()->trains[0]->train.value() == train1.lock());

  // Set path for train 2 from block 3 to block 2, that must fail:
  world->nxManager->select(nx3.lock(), nx2.lock());
  REQUIRE(block2.lock()->state == BlockState::Free);
  REQUIRE(block2.lock()->trains.size() == 0);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(block1.expired());
  REQUIRE(block2.expired());
  REQUIRE(block3.expired());
  REQUIRE(block4.expired());
  REQUIRE(nx1.expired());
  REQUIRE(nx2.expired());
  REQUIRE(nx3.expired());
  REQUIRE(nx4.expired());
  REQUIRE(cross.expired());
  REQUIRE(locomotive1.expired());
  REQUIRE(locomotive2.expired());
  REQUIRE(train1.expired());
  REQUIRE(train2.expired());
}

TEST_CASE("Board: Crossover path resevation using NX", "[board][board-path]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  // Board:
  // +--------+                     +--------+
  // | block1 |--(nx1)-\--/-(nx2)--| block2 |
  // +--------+         \/         +--------+
  // +--------+         /\         +--------+
  // | block3 |--(nx3)-/--\-(nx4)--| block4 |
  // +--------+                     +--------+
  std::weak_ptr<Board> boardWeak = world->boards->create();

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 0, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(2, 0, TileRotate::Deg90, TurnoutRight45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(3, 0, TileRotate::Deg270, TurnoutLeft45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(4, 0, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(5, 0, TileRotate::Deg90, BlockRailTile::classId, false));

  REQUIRE(boardWeak.lock()->addTile(0, 1, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 1, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(2, 1, TileRotate::Deg90, TurnoutLeft45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(3, 1, TileRotate::Deg270, TurnoutRight45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(4, 1, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(5, 1, TileRotate::Deg90, BlockRailTile::classId, false));

  std::weak_ptr<BlockRailTile> block1 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(block1.expired());
  std::weak_ptr<BlockRailTile> block2 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({5, 0}));
  REQUIRE_FALSE(block2.expired());
  std::weak_ptr<BlockRailTile> block3 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 1}));
  REQUIRE_FALSE(block3.expired());
  std::weak_ptr<BlockRailTile> block4 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({5, 1}));
  REQUIRE_FALSE(block4.expired());

  std::weak_ptr<NXButtonRailTile> nx1 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({1, 0}));
  REQUIRE_FALSE(nx1.expired());
  std::weak_ptr<NXButtonRailTile> nx2 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({4, 0}));
  REQUIRE_FALSE(nx2.expired());
  std::weak_ptr<NXButtonRailTile> nx3 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({1, 1}));
  REQUIRE_FALSE(nx3.expired());
  std::weak_ptr<NXButtonRailTile> nx4 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({4, 1}));
  REQUIRE_FALSE(nx4.expired());

  // Set blocks free:
  REQUIRE(block2.lock()->state == BlockState::Unknown);
  REQUIRE(block2.lock()->setStateFree());
  REQUIRE(block2.lock()->state == BlockState::Free);
  REQUIRE(block4.lock()->state == BlockState::Unknown);
  REQUIRE(block4.lock()->setStateFree());
  REQUIRE(block4.lock()->state == BlockState::Free);

  // Create two trains:
  std::weak_ptr<RailVehicle> locomotive1 = world->railVehicles->create(Locomotive::classId);
  std::weak_ptr<Train> train1 = world->trains->create();
  REQUIRE(train1.lock()->vehicles->length == 0);
  train1.lock()->vehicles->add(locomotive1.lock());
  REQUIRE(train1.lock()->vehicles->length == 1);

  std::weak_ptr<RailVehicle> locomotive2 = world->railVehicles->create(Locomotive::classId);
  std::weak_ptr<Train> train2 = world->trains->create();
  REQUIRE(train2.lock()->vehicles->length == 0);
  train2.lock()->vehicles->add(locomotive2.lock());
  REQUIRE(train2.lock()->vehicles->length == 1);

  // Assign train 1 to block 1:
  block1.lock()->assignTrain(train1.lock());
  REQUIRE(block1.lock()->state == BlockState::Reserved);
  block1.lock()->flipTrain();

  // Assign train 2 to block 3:
  block3.lock()->assignTrain(train2.lock());
  REQUIRE(block3.lock()->state == BlockState::Reserved);
  block3.lock()->flipTrain();

  // Set world in RUN state (required for selecting paths using NX buttons):
  world->run();

  REQUIRE(boardWeak.lock()->railCrossOver().find({2, 0}) != boardWeak.lock()->railCrossOver().end());

  // Set path for train 1 from block 1 to block 4:
  world->nxManager->select(nx1.lock(), nx4.lock());
  REQUIRE(block4.lock()->state == BlockState::Reserved);
  REQUIRE(block4.lock()->trains.size() == 1);
  REQUIRE(block4.lock()->trains[0]->train.value() == train1.lock());

  // Set path for train 2 from block 3 to block 2, that must fail:
  world->nxManager->select(nx3.lock(), nx2.lock());
  REQUIRE(block2.lock()->state.value() == BlockState::Free);
  REQUIRE(block2.lock()->trains.size() == 0);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(block1.expired());
  REQUIRE(block2.expired());
  REQUIRE(block3.expired());
  REQUIRE(block4.expired());
  REQUIRE(nx1.expired());
  REQUIRE(nx2.expired());
  REQUIRE(nx3.expired());
  REQUIRE(nx4.expired());
  REQUIRE(locomotive1.expired());
  REQUIRE(locomotive2.expired());
  REQUIRE(train1.expired());
  REQUIRE(train2.expired());
}

TEST_CASE("Board: Direction path reservation using NX and change direction state", "[board][board-path]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  // Board:
  // +--------+                     +--------+
  // | block1 |--(nx1)--(-)--(nx2)--| block2 |
  // +--------+                     +--------+
  std::weak_ptr<Board> boardWeak = world->boards->create();

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 0, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(2, 0, TileRotate::Deg90, DirectionControlRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(3, 0, TileRotate::Deg90, NXButtonRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(4, 0, TileRotate::Deg90, BlockRailTile::classId, false));

  std::weak_ptr<BlockRailTile> block1 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(block1.expired());
  std::weak_ptr<BlockRailTile> block2 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({4, 0}));
  REQUIRE_FALSE(block2.expired());

  std::weak_ptr<NXButtonRailTile> nx1 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({1, 0}));
  REQUIRE_FALSE(nx1.expired());
  std::weak_ptr<NXButtonRailTile> nx2 = std::dynamic_pointer_cast<NXButtonRailTile>(boardWeak.lock()->getTile({3, 0}));
  REQUIRE_FALSE(nx2.expired());

  std::weak_ptr<DirectionControlRailTile> directionControl = std::dynamic_pointer_cast<DirectionControlRailTile>(boardWeak.lock()->getTile({2, 0}));
  REQUIRE_FALSE(directionControl.expired());
  REQUIRE(directionControl.lock()->state.value() == DirectionControlState::Both);

  // Change direction control states (no reservation, so all valuese are allowed):
  REQUIRE(directionControl.lock()->setState(DirectionControlState::AtoB));
  REQUIRE(directionControl.lock()->state.value() == DirectionControlState::AtoB);
  REQUIRE(directionControl.lock()->setState(DirectionControlState::BtoA));
  REQUIRE(directionControl.lock()->state.value() == DirectionControlState::BtoA);
  REQUIRE(directionControl.lock()->setState(DirectionControlState::None));
  REQUIRE(directionControl.lock()->state.value() == DirectionControlState::None);
  REQUIRE(directionControl.lock()->setState(DirectionControlState::Both));
  REQUIRE(directionControl.lock()->state.value() == DirectionControlState::Both);

  // Set block free:
  REQUIRE(block2.lock()->state == BlockState::Unknown);
  REQUIRE(block2.lock()->setStateFree());
  REQUIRE(block2.lock()->state == BlockState::Free);

  // Create train:
  std::weak_ptr<RailVehicle> locomotive = world->railVehicles->create(Locomotive::classId);
  std::weak_ptr<Train> train = world->trains->create();
  REQUIRE(train.lock()->vehicles->length == 0);
  train.lock()->vehicles->add(locomotive.lock());
  REQUIRE(train.lock()->vehicles->length == 1);

  // Assign train to block 1:
  block1.lock()->assignTrain(train.lock());
  REQUIRE(block1.lock()->state == BlockState::Reserved);
  block1.lock()->flipTrain();

  // Set world in RUN state (required for selecting paths using NX buttons):
  world->run();

  // Set path for train from block 1 to block 2:
  world->nxManager->select(nx1.lock(), nx2.lock());
  REQUIRE(block2.lock()->state == BlockState::Reserved);
  REQUIRE(block2.lock()->trains.size() == 1);
  REQUIRE(block2.lock()->trains[0]->train.value() == train.lock());

  // Change direction control state:

  // A -> B = allowed as that is the train direction.
  REQUIRE(directionControl.lock()->setState(DirectionControlState::AtoB));
  REQUIRE(directionControl.lock()->state.value() == DirectionControlState::AtoB);

  // B -> A = not allowed as that is opposite of the the train direction.
  REQUIRE_FALSE(directionControl.lock()->setState(DirectionControlState::BtoA));
  REQUIRE(directionControl.lock()->state.value() == DirectionControlState::AtoB);

  // None = not allowed when reserved.
  REQUIRE_FALSE(directionControl.lock()->setState(DirectionControlState::None));
  REQUIRE(directionControl.lock()->state.value() == DirectionControlState::AtoB);

  // Both = always allowed.
  REQUIRE(directionControl.lock()->setState(DirectionControlState::Both));
  REQUIRE(directionControl.lock()->state.value() == DirectionControlState::Both);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(block1.expired());
  REQUIRE(block2.expired());
  REQUIRE(nx1.expired());
  REQUIRE(nx2.expired());
  REQUIRE(directionControl.expired());
  REQUIRE(locomotive.expired());
  REQUIRE(train.expired());
}
