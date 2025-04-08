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
#include "../src/zone/zonelist.hpp"
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
