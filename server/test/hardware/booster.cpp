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

#include <catch2/catch_template_test_macros.hpp>
#include <ranges>
#include "../src/core/method.tpp"
#include "../src/core/objectproperty.tpp"
#include "../src/hardware/booster/drivers/boosterdrivers.hpp"
#include "../src/hardware/booster/list/boosterlist.hpp"
#include "../src/world/world.hpp"

TEST_CASE("Create world and booster => destroy world", "[object-create-destroy]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(worldWeak.lock()->boosters->length == 0);

  std::weak_ptr<Booster> boosterWeak = world->boosters->create();
  REQUIRE_FALSE(boosterWeak.expired());
  std::weak_ptr<BoosterDriver> driverWeak = boosterWeak.lock()->driver.value();
  REQUIRE_FALSE(driverWeak.expired());
  REQUIRE(boosterWeak.lock()->getClassId() == Booster::classId);
  REQUIRE(worldWeak.lock()->boosters->length == 1);

  world.reset();
  REQUIRE(boosterWeak.expired());
  REQUIRE(driverWeak.expired());
  REQUIRE(worldWeak.expired());
}

TEST_CASE("Create world and booster => destroy booster", "[object-create-destroy]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(worldWeak.lock()->boosters->length == 0);

  std::weak_ptr<Booster> boosterWeak = world->boosters->create();
  REQUIRE_FALSE(boosterWeak.expired());
  std::weak_ptr<BoosterDriver> driverWeak = boosterWeak.lock()->driver.value();
  REQUIRE_FALSE(driverWeak.expired());
  REQUIRE(worldWeak.lock()->boosters->length == 1);

  world->boosters->delete_(boosterWeak.lock());
  REQUIRE(driverWeak.expired());
  REQUIRE(boosterWeak.expired());
  REQUIRE(worldWeak.lock()->boosters->length == 0);

  world.reset();
  REQUIRE(worldWeak.expired());
}

TEST_CASE("Booster change driver", "[object-create-destroy]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(worldWeak.lock()->boosters->length == 0);

  std::weak_ptr<Booster> boosterWeak = world->boosters->create();
  REQUIRE_FALSE(boosterWeak.expired());
  std::weak_ptr<BoosterDriver> driverWeak = boosterWeak.lock()->driver.value();
  REQUIRE_FALSE(driverWeak.expired());
  REQUIRE(worldWeak.lock()->boosters->length == 1);

  for(auto type : BoosterDrivers::types() | std::views::reverse)
  {
    boosterWeak.lock()->type = std::string(type);
    REQUIRE(driverWeak.expired());
    driverWeak = boosterWeak.lock()->driver.value();
    REQUIRE_FALSE(driverWeak.expired());
  }

  world.reset();
  REQUIRE(boosterWeak.expired());
  REQUIRE(driverWeak.expired());
  REQUIRE(worldWeak.expired());
}

