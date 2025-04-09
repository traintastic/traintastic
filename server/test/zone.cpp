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

TEST_CASE("Zone: Assign/remove train to/from muted and no smoke zone", "[zone]")
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
  REQUIRE(zoneWeak.lock()->mute);
  REQUIRE(zoneWeak.lock()->noSmoke);
  REQUIRE(zoneWeak.lock()->blocks->length == 0);
  zoneWeak.lock()->blocks->add(blockWeak.lock());
  REQUIRE(zoneWeak.lock()->blocks->length == 1);
  REQUIRE(zoneWeak.lock()->trains.size() == 0);

  // Assign train to block in muted and no smoke zone:
  REQUIRE_FALSE(trainWeak.lock()->active);
  REQUIRE_FALSE(trainWeak.lock()->mute);
  REQUIRE_FALSE(trainWeak.lock()->noSmoke);
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
  REQUIRE(locomotiveWeak.lock()->mute);
  REQUIRE(locomotiveWeak.lock()->noSmoke);

  // Remove train from block in muted and no smoke zone:
  blockWeak.lock()->removeTrain(trainWeak.lock());
  REQUIRE_FALSE(trainWeak.lock()->active);
  REQUIRE(trainWeak.lock()->blocks.size() == 0);
  REQUIRE(trainWeak.lock()->zones.size() == 0);
  REQUIRE(blockWeak.lock()->trains.size() == 0);
  REQUIRE(zoneWeak.lock()->trains.size() == 0);
  REQUIRE_FALSE(trainWeak.lock()->mute);
  REQUIRE_FALSE(trainWeak.lock()->noSmoke);
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

TEST_CASE("Zone: Toggle mute/noSmoke with train in zone", "[zone]")
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

  zoneWeak.lock()->mute = true;
  zoneWeak.lock()->noSmoke = true;

  blockWeak.lock()->removeTrain(trainWeak.lock());

  REQUIRE_FALSE(trainWeak.lock()->mute);
  REQUIRE_FALSE(locomotiveWeak.lock()->mute);
  REQUIRE_FALSE(trainWeak.lock()->noSmoke);
  REQUIRE_FALSE(locomotiveWeak.lock()->noSmoke);

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
