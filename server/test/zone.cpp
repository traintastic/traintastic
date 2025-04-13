/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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
#include <catch2/catch_approx.hpp>
#include "../src/core/attributes.hpp"
#include "../src/core/method.tpp"
#include "../src/core/objectproperty.tpp"
#include "../src/board/board.hpp"
#include "../src/board/boardlist.hpp"
#include "../src/board/tile/rail/blockrailtile.hpp"
#include "../src/world/world.hpp"
#include "../src/vehicle/rail/railvehiclelist.hpp"
#include "../src/vehicle/rail/railvehiclelisttablemodel.hpp"
#include "../src/vehicle/rail/locomotive.hpp"
#include "../src/train/trainlist.hpp"
#include "../src/train/train.hpp"
#include "../src/train/trainvehiclelist.hpp"
#include "../src/hardware/decoder/decoder.hpp"
#include "../src/zone/blockzonelist.hpp"
#include "../src/zone/zonelist.hpp"
#include "../src/zone/zonelisttablemodel.hpp"
#include "../src/zone/zone.hpp"
#include "../src/zone/zoneblocklist.hpp"

TEST_CASE("Zone: Assign/remove train to/from muted, no smoke and speed limited zone", "[zone]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());

  REQUIRE(world->railVehicles->length == 0);
  std::weak_ptr<RailVehicle> locomotiveWeak = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak.expired());
  REQUIRE(world->railVehicles->length == 1);

  REQUIRE(world->trains->length == 0);
  std::weak_ptr<Train> trainWeak = world->trains->create();
  REQUIRE_FALSE(trainWeak.expired());
  REQUIRE(world->trains->length == 1);
  REQUIRE(trainWeak.lock()->vehicles->length == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  REQUIRE(trainWeak.lock()->vehicles->length == 1);

  REQUIRE(world->boards->length == 0);
  std::weak_ptr<Board> boardWeak = world->boards->create();
  REQUIRE_FALSE(boardWeak.expired());
  REQUIRE(world->boards->length == 1);

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  std::weak_ptr<BlockRailTile> blockWeak = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(blockWeak.expired());

  std::weak_ptr<Zone> zoneWeak = world->zones->create();
  REQUIRE_FALSE(zoneWeak.expired());
  REQUIRE_FALSE(zoneWeak.lock()->mute.value());
  REQUIRE_FALSE(zoneWeak.lock()->noSmoke.value());
  zoneWeak.lock()->mute = true;
  zoneWeak.lock()->noSmoke = true;
  zoneWeak.lock()->speedLimit.setValue(100.0);
  REQUIRE(zoneWeak.lock()->mute);
  REQUIRE(zoneWeak.lock()->noSmoke);
  REQUIRE(zoneWeak.lock()->speedLimit.value() == Catch::Approx(100.0));
  REQUIRE(zoneWeak.lock()->blocks->length == 0);
  zoneWeak.lock()->blocks->add(blockWeak.lock());
  REQUIRE(zoneWeak.lock()->blocks->length == 1);
  REQUIRE(zoneWeak.lock()->trains.size() == 0);

  // Assign train to block in muted, no smoke and speed limited zone:
  REQUIRE_FALSE(trainWeak.lock()->active);
  REQUIRE_FALSE(trainWeak.lock()->mute);
  REQUIRE_FALSE(trainWeak.lock()->noSmoke);
  REQUIRE(std::isinf(trainWeak.lock()->speedLimit.value()));
  REQUIRE_FALSE(locomotiveWeak.lock()->mute);
  REQUIRE_FALSE(locomotiveWeak.lock()->noSmoke);
  blockWeak.lock()->assignTrain(trainWeak.lock());
  REQUIRE(trainWeak.lock()->active);
  REQUIRE(trainWeak.lock()->blocks.size() == 1);
  REQUIRE(trainWeak.lock()->zones.size() == 1);
  REQUIRE(blockWeak.lock()->trains.size() == 1);
  REQUIRE(zoneWeak.lock()->trains.size() == 1);
  REQUIRE(trainWeak.lock()->mute);
  REQUIRE(trainWeak.lock()->noSmoke);
  REQUIRE(trainWeak.lock()->speedLimit.value() == Catch::Approx(100.0));
  REQUIRE(locomotiveWeak.lock()->mute);
  REQUIRE(locomotiveWeak.lock()->noSmoke);

  // Remove train from block in muted, no smoke and speed limited zone:
  blockWeak.lock()->removeTrain(trainWeak.lock());
  REQUIRE_FALSE(trainWeak.lock()->active);
  REQUIRE(trainWeak.lock()->blocks.size() == 0);
  REQUIRE(trainWeak.lock()->zones.size() == 0);
  REQUIRE(blockWeak.lock()->trains.size() == 0);
  REQUIRE(zoneWeak.lock()->trains.size() == 0);
  REQUIRE_FALSE(trainWeak.lock()->mute);
  REQUIRE_FALSE(trainWeak.lock()->noSmoke);
  REQUIRE(std::isinf(trainWeak.lock()->speedLimit.value()));
  REQUIRE_FALSE(locomotiveWeak.lock()->mute);
  REQUIRE_FALSE(locomotiveWeak.lock()->noSmoke);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(locomotiveWeak.expired());
  REQUIRE(trainWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(blockWeak.expired());
  REQUIRE(zoneWeak.expired());
}

TEST_CASE("Zone: Assign/remove events", "[zone]")
{
  size_t trainZoneAssignedEventCount = 0;
  size_t trainZoneEnteringEventCount = 0;
  size_t trainZoneEnteredEventCount = 0;
  size_t trainZoneLeavingEventCount = 0;
  size_t trainZoneLeftEventCount = 0;
  size_t trainZoneRemovedEventCount = 0;
  size_t zoneTrainAssignedEventCount = 0;
  size_t zoneTrainEnteringEventCount = 0;
  size_t zoneTrainEnteredEventCount = 0;
  size_t zoneTrainLeavingEventCount = 0;
  size_t zoneTrainLeftEventCount = 0;
  size_t zoneTrainRemovedEventCount = 0;

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());

  REQUIRE(world->railVehicles->length == 0);
  std::weak_ptr<RailVehicle> locomotiveWeak = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak.expired());
  REQUIRE(world->railVehicles->length == 1);

  REQUIRE(world->trains->length == 0);
  std::weak_ptr<Train> trainWeak = world->trains->create();
  REQUIRE_FALSE(trainWeak.expired());
  REQUIRE(world->trains->length == 1);
  REQUIRE(trainWeak.lock()->vehicles->length == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  REQUIRE(trainWeak.lock()->vehicles->length == 1);

  REQUIRE(world->boards->length == 0);
  std::weak_ptr<Board> boardWeak = world->boards->create();
  REQUIRE_FALSE(boardWeak.expired());
  REQUIRE(world->boards->length == 1);

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  std::weak_ptr<BlockRailTile> blockWeak = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(blockWeak.expired());

  std::weak_ptr<Zone> zoneWeak = world->zones->create();
  REQUIRE_FALSE(zoneWeak.expired());
  REQUIRE(zoneWeak.lock()->blocks->length == 0);
  zoneWeak.lock()->blocks->add(blockWeak.lock());
  REQUIRE(zoneWeak.lock()->blocks->length == 1);
  REQUIRE(zoneWeak.lock()->trains.size() == 0);

  // Setup events:
  trainWeak.lock()->onZoneAssigned.connect(
    [&](const std::shared_ptr<Train>& train, const std::shared_ptr<Zone>& zone)
    {
      trainZoneAssignedEventCount++;
      REQUIRE(train == trainWeak.lock());
      REQUIRE(zone == zoneWeak.lock());
    });
  trainWeak.lock()->onZoneEntering.connect(
    [&](const std::shared_ptr<Train>& train, const std::shared_ptr<Zone>& zone)
    {
      trainZoneEnteringEventCount++;
      REQUIRE(train == trainWeak.lock());
      REQUIRE(zone == zoneWeak.lock());
    });
  trainWeak.lock()->onZoneEntered.connect(
    [&](const std::shared_ptr<Train>& train, const std::shared_ptr<Zone>& zone)
    {
      trainZoneEnteredEventCount++;
      REQUIRE(train == trainWeak.lock());
      REQUIRE(zone == zoneWeak.lock());
    });
  trainWeak.lock()->onZoneLeaving.connect(
    [&](const std::shared_ptr<Train>& train, const std::shared_ptr<Zone>& zone)
    {
      trainZoneLeavingEventCount++;
      REQUIRE(train == trainWeak.lock());
      REQUIRE(zone == zoneWeak.lock());
    });
  trainWeak.lock()->onZoneLeft.connect(
    [&](const std::shared_ptr<Train>& train, const std::shared_ptr<Zone>& zone)
    {
      trainZoneLeftEventCount++;
      REQUIRE(train == trainWeak.lock());
      REQUIRE(zone == zoneWeak.lock());
    });
  trainWeak.lock()->onZoneRemoved.connect(
    [&](const std::shared_ptr<Train>& train, const std::shared_ptr<Zone>& zone)
    {
      trainZoneRemovedEventCount++;
      REQUIRE(train == trainWeak.lock());
      REQUIRE(zone == zoneWeak.lock());
    });
  zoneWeak.lock()->onTrainAssigned.connect(
    [&](const std::shared_ptr<Train>& train, const std::shared_ptr<Zone>& zone)
    {
      zoneTrainAssignedEventCount++;
      REQUIRE(train == trainWeak.lock());
      REQUIRE(zone == zoneWeak.lock());
    });
  zoneWeak.lock()->onTrainEntering.connect(
    [&](const std::shared_ptr<Train>& train, const std::shared_ptr<Zone>& zone)
    {
      zoneTrainEnteringEventCount++;
      REQUIRE(train == trainWeak.lock());
      REQUIRE(zone == zoneWeak.lock());
    });
  zoneWeak.lock()->onTrainEntered.connect(
    [&](const std::shared_ptr<Train>& train, const std::shared_ptr<Zone>& zone)
    {
      zoneTrainEnteredEventCount++;
      REQUIRE(train == trainWeak.lock());
      REQUIRE(zone == zoneWeak.lock());
    });
  zoneWeak.lock()->onTrainLeaving.connect(
    [&](const std::shared_ptr<Train>& train, const std::shared_ptr<Zone>& zone)
    {
      zoneTrainLeavingEventCount++;
      REQUIRE(train == trainWeak.lock());
      REQUIRE(zone == zoneWeak.lock());
    });
  zoneWeak.lock()->onTrainLeft.connect(
    [&](const std::shared_ptr<Train>& train, const std::shared_ptr<Zone>& zone)
    {
      zoneTrainLeftEventCount++;
      REQUIRE(train == trainWeak.lock());
      REQUIRE(zone == zoneWeak.lock());
    });
  zoneWeak.lock()->onTrainRemoved.connect(
    [&](const std::shared_ptr<Train>& train, const std::shared_ptr<Zone>& zone)
    {
      zoneTrainRemovedEventCount++;
      REQUIRE(train == trainWeak.lock());
      REQUIRE(zone == zoneWeak.lock());
    });

  REQUIRE(trainZoneAssignedEventCount == 0);
  REQUIRE(trainZoneEnteringEventCount == 0);
  REQUIRE(trainZoneEnteredEventCount == 0);
  REQUIRE(trainZoneLeavingEventCount == 0);
  REQUIRE(trainZoneLeftEventCount == 0);
  REQUIRE(trainZoneRemovedEventCount == 0);
  REQUIRE(zoneTrainAssignedEventCount == 0);
  REQUIRE(zoneTrainEnteringEventCount == 0);
  REQUIRE(zoneTrainEnteredEventCount == 0);
  REQUIRE(zoneTrainLeavingEventCount == 0);
  REQUIRE(zoneTrainLeftEventCount == 0);
  REQUIRE(zoneTrainRemovedEventCount == 0);

  blockWeak.lock()->assignTrain(trainWeak.lock());

  REQUIRE(trainZoneAssignedEventCount == 1);
  REQUIRE(trainZoneEnteringEventCount == 0);
  REQUIRE(trainZoneEnteredEventCount == 0);
  REQUIRE(trainZoneLeavingEventCount == 0);
  REQUIRE(trainZoneLeftEventCount == 0);
  REQUIRE(trainZoneRemovedEventCount == 0);
  REQUIRE(zoneTrainAssignedEventCount == 1);
  REQUIRE(zoneTrainEnteringEventCount == 0);
  REQUIRE(zoneTrainEnteredEventCount == 0);
  REQUIRE(zoneTrainLeavingEventCount == 0);
  REQUIRE(zoneTrainLeftEventCount == 0);
  REQUIRE(zoneTrainRemovedEventCount == 0);

  blockWeak.lock()->removeTrain(trainWeak.lock());

  REQUIRE(trainZoneAssignedEventCount == 1);
  REQUIRE(trainZoneEnteringEventCount == 0);
  REQUIRE(trainZoneEnteredEventCount == 0);
  REQUIRE(trainZoneLeavingEventCount == 0);
  REQUIRE(trainZoneLeftEventCount == 0);
  REQUIRE(trainZoneRemovedEventCount == 1);
  REQUIRE(zoneTrainAssignedEventCount == 1);
  REQUIRE(zoneTrainEnteringEventCount == 0);
  REQUIRE(zoneTrainEnteredEventCount == 0);
  REQUIRE(zoneTrainLeavingEventCount == 0);
  REQUIRE(zoneTrainLeftEventCount == 0);
  REQUIRE(zoneTrainRemovedEventCount == 1);

  world.reset();

  REQUIRE(worldWeak.expired());
  REQUIRE(locomotiveWeak.expired());
  REQUIRE(trainWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(blockWeak.expired());
  REQUIRE(zoneWeak.expired());
  REQUIRE(trainZoneAssignedEventCount == 1);
  REQUIRE(trainZoneEnteringEventCount == 0);
  REQUIRE(trainZoneEnteredEventCount == 0);
  REQUIRE(trainZoneLeavingEventCount == 0);
  REQUIRE(trainZoneLeftEventCount == 0);
  REQUIRE(trainZoneRemovedEventCount == 1);
  REQUIRE(zoneTrainAssignedEventCount == 1);
  REQUIRE(zoneTrainEnteringEventCount == 0);
  REQUIRE(zoneTrainEnteredEventCount == 0);
  REQUIRE(zoneTrainLeavingEventCount == 0);
  REQUIRE(zoneTrainLeftEventCount == 0);
  REQUIRE(zoneTrainRemovedEventCount == 1);
}

TEST_CASE("Zone: Toggle mute/noSmoke/speedLimit with train in zone", "[zone]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());

  REQUIRE(world->railVehicles->length == 0);
  std::weak_ptr<RailVehicle> locomotiveWeak = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak.expired());
  REQUIRE(world->railVehicles->length == 1);

  REQUIRE(world->trains->length == 0);
  std::weak_ptr<Train> trainWeak = world->trains->create();
  REQUIRE_FALSE(trainWeak.expired());
  REQUIRE(world->trains->length == 1);
  REQUIRE(trainWeak.lock()->vehicles->length == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  REQUIRE(trainWeak.lock()->vehicles->length == 1);

  REQUIRE(world->boards->length == 0);
  std::weak_ptr<Board> boardWeak = world->boards->create();
  REQUIRE_FALSE(boardWeak.expired());
  REQUIRE(world->boards->length == 1);

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  std::weak_ptr<BlockRailTile> blockWeak = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(blockWeak.expired());

  std::weak_ptr<Zone> zoneWeak = world->zones->create();
  REQUIRE_FALSE(zoneWeak.expired());
  REQUIRE(zoneWeak.lock()->blocks->length == 0);
  zoneWeak.lock()->blocks->add(blockWeak.lock());
  REQUIRE(zoneWeak.lock()->blocks->length == 1);
  REQUIRE(zoneWeak.lock()->trains.size() == 0);

  REQUIRE_FALSE(trainWeak.lock()->active);
  blockWeak.lock()->assignTrain(trainWeak.lock());
  REQUIRE(trainWeak.lock()->active);
  REQUIRE(trainWeak.lock()->blocks.size() == 1);
  REQUIRE(trainWeak.lock()->zones.size() == 1);
  REQUIRE(blockWeak.lock()->trains.size() == 1);
  REQUIRE(zoneWeak.lock()->trains.size() == 1);

  REQUIRE_FALSE(trainWeak.lock()->mute);
  REQUIRE_FALSE(trainWeak.lock()->noSmoke);
  REQUIRE_FALSE(locomotiveWeak.lock()->mute);
  REQUIRE_FALSE(locomotiveWeak.lock()->noSmoke);

  zoneWeak.lock()->mute = true;
  REQUIRE(trainWeak.lock()->mute);
  REQUIRE(locomotiveWeak.lock()->mute);

  zoneWeak.lock()->mute = false;
  REQUIRE_FALSE(trainWeak.lock()->mute);
  REQUIRE_FALSE(locomotiveWeak.lock()->mute);

  zoneWeak.lock()->noSmoke = true;
  REQUIRE(trainWeak.lock()->noSmoke);
  REQUIRE(locomotiveWeak.lock()->noSmoke);

  zoneWeak.lock()->noSmoke = false;
  REQUIRE_FALSE(trainWeak.lock()->noSmoke);
  REQUIRE_FALSE(locomotiveWeak.lock()->noSmoke);

  zoneWeak.lock()->speedLimit.setValue(100.0);
  REQUIRE(trainWeak.lock()->speedLimit.value() == Catch::Approx(100.0));

  zoneWeak.lock()->speedLimit.setValue(std::numeric_limits<double>::infinity());
  REQUIRE(std::isinf(trainWeak.lock()->speedLimit.value()));

  zoneWeak.lock()->mute = true;
  zoneWeak.lock()->noSmoke = true;
  zoneWeak.lock()->speedLimit.setValue(100.0);

  blockWeak.lock()->removeTrain(trainWeak.lock());

  REQUIRE_FALSE(trainWeak.lock()->mute);
  REQUIRE_FALSE(locomotiveWeak.lock()->mute);
  REQUIRE_FALSE(trainWeak.lock()->noSmoke);
  REQUIRE_FALSE(locomotiveWeak.lock()->noSmoke);
  REQUIRE(std::isinf(trainWeak.lock()->speedLimit.value()));

  world.reset();

  REQUIRE(worldWeak.expired());
  REQUIRE(locomotiveWeak.expired());
  REQUIRE(trainWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(blockWeak.expired());
  REQUIRE(zoneWeak.expired());
}

TEST_CASE("Zone: Check class id's", "[zone]")
{
  // class id's may NOT be changed, it will break saved worlds and might break client stuff.

  REQUIRE(BlockZoneList::classId == "list.block_zone");
  REQUIRE(Zone::classId == "zone");
  REQUIRE(ZoneBlockList::classId == "list.zone_block");
  REQUIRE(ZoneList::classId == "list.zone");
  REQUIRE(ZoneListTableModel::classId == "zone_list_table_model");

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(world->zones->getClassId() == "list.zone");

  std::weak_ptr<Board> boardWeak = world->boards->create();
  REQUIRE_FALSE(boardWeak.expired());

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  std::weak_ptr<BlockRailTile> blockWeak = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(blockWeak.expired());
  REQUIRE(blockWeak.lock()->zones->getClassId() == "list.block_zone");

  std::weak_ptr<Zone> zoneWeak = world->zones->create();
  REQUIRE_FALSE(zoneWeak.expired());
  REQUIRE(zoneWeak.lock()->getClassId() == "zone");
  REQUIRE(zoneWeak.lock()->blocks->getClassId() == "list.zone_block");

  world.reset();

  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(blockWeak.expired());
  REQUIRE(zoneWeak.expired());
}

TEST_CASE("Zone: Check enabled attribute", "[zone]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());

  std::weak_ptr<Zone> zoneWeak = world->zones->create();
  REQUIRE_FALSE(zoneWeak.expired());

  REQUIRE(world->state.value() == static_cast<WorldState>(0));
  REQUIRE_FALSE(world->edit);
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->id));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->name));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->mute));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->noSmoke));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->speedLimit));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->blocks->add));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->blocks->remove));

  world->edit = true;

  REQUIRE(world->state.value() == WorldState::Edit);
  REQUIRE(world->edit);
  REQUIRE(Attributes::getEnabled(zoneWeak.lock()->id));
  REQUIRE(Attributes::getEnabled(zoneWeak.lock()->name));
  REQUIRE(Attributes::getEnabled(zoneWeak.lock()->mute));
  REQUIRE(Attributes::getEnabled(zoneWeak.lock()->noSmoke));
  REQUIRE(Attributes::getEnabled(zoneWeak.lock()->speedLimit));
  REQUIRE(Attributes::getEnabled(zoneWeak.lock()->blocks->add));
  REQUIRE(Attributes::getEnabled(zoneWeak.lock()->blocks->remove));

  world->run();

  REQUIRE(world->state.value() == (WorldState::Edit | WorldState::PowerOn | WorldState::Run));
  REQUIRE(world->edit);
  REQUIRE(Attributes::getEnabled(zoneWeak.lock()->id));
  REQUIRE(Attributes::getEnabled(zoneWeak.lock()->name));
  REQUIRE(Attributes::getEnabled(zoneWeak.lock()->mute));
  REQUIRE(Attributes::getEnabled(zoneWeak.lock()->noSmoke));
  REQUIRE(Attributes::getEnabled(zoneWeak.lock()->speedLimit));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->blocks->add));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->blocks->remove));

  world->edit = false;

  REQUIRE(world->state.value() == (WorldState::PowerOn | WorldState::Run));
  REQUIRE_FALSE(world->edit);
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->id));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->name));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->mute));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->noSmoke));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->speedLimit));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->blocks->add));
  REQUIRE_FALSE(Attributes::getEnabled(zoneWeak.lock()->blocks->remove));

  world.reset();

  REQUIRE(worldWeak.expired());
  REQUIRE(zoneWeak.expired());
}

TEST_CASE("Zone: zone list table model", "[zone]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());

  auto zoneListModel = world->zones->getModel();
  REQUIRE(zoneListModel);
  REQUIRE(zoneListModel->getClassId() == "zone_list_table_model");
  REQUIRE(zoneListModel->rowCount() == 0);

  std::weak_ptr<Zone> zone1 = world->zones->create();
  REQUIRE_FALSE(zone1.expired());

  REQUIRE(zoneListModel->rowCount() == 1);
  REQUIRE(zoneListModel->getText(0, 0) == zone1.lock()->id.value());
  REQUIRE(zoneListModel->getText(1, 0) == zone1.lock()->name.value());

  zone1.lock()->id = "zone_one";
  REQUIRE(zoneListModel->getText(0, 0) == "zone_one");
  zone1.lock()->name = "Zone One";
  REQUIRE(zoneListModel->getText(1, 0) == "Zone One");

  std::weak_ptr<Zone> zone2 = world->zones->create();
  REQUIRE_FALSE(zone2.expired());

  REQUIRE(zoneListModel->rowCount() == 2);
  REQUIRE(zoneListModel->getText(0, 1) == zone2.lock()->id.value());
  REQUIRE(zoneListModel->getText(1, 1) == zone2.lock()->name.value());

  zone2.lock()->id = "zone_two";
  REQUIRE(zoneListModel->getText(0, 1) == "zone_two");
  zone2.lock()->name = "Zone Two";
  REQUIRE(zoneListModel->getText(1, 1) == "Zone Two");

  world->zones->delete_(zone1.lock());
  REQUIRE(zone1.expired());

  REQUIRE(zoneListModel->rowCount() == 1);
  REQUIRE(zoneListModel->getText(0, 0) == "zone_two");
  REQUIRE(zoneListModel->getText(1, 0) == "Zone Two");
  REQUIRE(zoneListModel->getText(0, 1) == "");
  REQUIRE(zoneListModel->getText(1, 1) == "");

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(zone2.expired());
}

TEST_CASE("Zone: block zone list table model", "[zone]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());

  std::weak_ptr<Board> boardWeak = world->boards->create();
  REQUIRE_FALSE(boardWeak.expired());

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  std::weak_ptr<BlockRailTile> blockWeak = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(blockWeak.expired());

  std::weak_ptr<Zone> zone1 = world->zones->create();
  REQUIRE_FALSE(zone1.expired());

  std::weak_ptr<Zone> zone2 = world->zones->create();
  REQUIRE_FALSE(zone2.expired());

  auto blockZoneList = blockWeak.lock()->zones->getModel();
  REQUIRE(blockZoneList->rowCount() == 0);

  blockWeak.lock()->zones->add(zone1.lock());
  REQUIRE(blockZoneList->rowCount() == 1);
  REQUIRE(blockZoneList->getText(0, 0) == zone1.lock()->id.value());
  REQUIRE(blockZoneList->getText(1, 0) == zone1.lock()->name.value());

  zone1.lock()->id = "zone_one";
  zone1.lock()->name = "Zone One";
  REQUIRE(blockZoneList->getText(0, 0) == "zone_one");
  REQUIRE(blockZoneList->getText(1, 0) == "Zone One");

  zone2.lock()->blocks->add(blockWeak.lock());
  REQUIRE(blockZoneList->rowCount() == 2);
  REQUIRE(blockZoneList->getText(0, 1) == zone2.lock()->id.value());
  REQUIRE(blockZoneList->getText(1, 1) == zone2.lock()->name.value());

  zone2.lock()->id = "zone_two";
  zone2.lock()->name = "Zone Two";
  REQUIRE(blockZoneList->getText(0, 1) == "zone_two");
  REQUIRE(blockZoneList->getText(1, 1) == "Zone Two");

  blockWeak.lock()->zones->remove(zone1.lock());
  REQUIRE(blockZoneList->rowCount() == 1);
  REQUIRE(blockZoneList->getText(0, 0) == "zone_two");
  REQUIRE(blockZoneList->getText(1, 0) == "Zone Two");

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(blockWeak.expired());
  REQUIRE(zone1.expired());
  REQUIRE(zone2.expired());
}

TEST_CASE("!Zone: delete zone with block assigned", "[zone]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());

  std::weak_ptr<Board> boardWeak = world->boards->create();
  REQUIRE_FALSE(boardWeak.expired());

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  std::weak_ptr<BlockRailTile> blockWeak = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(blockWeak.expired());

  std::weak_ptr<Zone> zoneWeak = world->zones->create();
  REQUIRE_FALSE(zoneWeak.expired());

  zoneWeak.lock()->blocks->add(blockWeak.lock());
  REQUIRE(zoneWeak.lock()->blocks->length.value() == 1);
  REQUIRE(blockWeak.lock()->zones->length.value() == 1);

  world->zones->delete_(zoneWeak.lock());
  REQUIRE(zoneWeak.expired());
  REQUIRE(world->zones->length.value() == 0);
  REQUIRE(blockWeak.lock()->zones->length.value() == 0);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(blockWeak.expired());
}

TEST_CASE("!Zone: delete block with zone assigned", "[zone]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());

  std::weak_ptr<Board> boardWeak = world->boards->create();
  REQUIRE_FALSE(boardWeak.expired());

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  std::weak_ptr<BlockRailTile> blockWeak = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(blockWeak.expired());

  std::weak_ptr<Zone> zoneWeak = world->zones->create();
  REQUIRE_FALSE(zoneWeak.expired());

  zoneWeak.lock()->blocks->add(blockWeak.lock());
  REQUIRE(zoneWeak.lock()->blocks->length.value() == 1);
  REQUIRE(blockWeak.lock()->zones->length.value() == 1);

  boardWeak.lock()->deleteTile(0, 0);
  REQUIRE(blockWeak.expired());
  REQUIRE(zoneWeak.lock()->blocks->length.value() == 0);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(zoneWeak.expired());
}

TEST_CASE("!Zone: delete board with block with zone assigned", "[zone]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());

  std::weak_ptr<Board> boardWeak = world->boards->create();
  REQUIRE_FALSE(boardWeak.expired());

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  std::weak_ptr<BlockRailTile> blockWeak = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(blockWeak.expired());

  std::weak_ptr<Zone> zoneWeak = world->zones->create();
  REQUIRE_FALSE(zoneWeak.expired());

  zoneWeak.lock()->blocks->add(blockWeak.lock());
  REQUIRE(zoneWeak.lock()->blocks->length.value() == 1);
  REQUIRE(blockWeak.lock()->zones->length.value() == 1);

  world->boards->delete_(boardWeak.lock());
  REQUIRE(boardWeak.expired());
  REQUIRE(blockWeak.expired());
  REQUIRE(zoneWeak.lock()->blocks->length.value() == 0);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(zoneWeak.expired());
}
