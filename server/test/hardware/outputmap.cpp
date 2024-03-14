/**
 * server/test/hardware/outputmap.cpp
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

#include <catch2/catch.hpp>
#include "../../src/core/method.tpp"
#include "../../src/core/objectproperty.tpp"
#include "../../src/world/world.hpp"
#include "../../src/hardware/interface/interfacelist.hpp"
#include "../../src/hardware/interface/ecosinterface.hpp"
#include "../../src/hardware/interface/loconetinterface.hpp"
#include "../../src/hardware/interface/xpressnetinterface.hpp"
#include "../../src/hardware/output/map/outputmappairoutputaction.hpp"
#include "../../src/hardware/output/map/outputmapecosstateoutputaction.hpp"
#include "../../src/board/board.hpp"
#include "../../src/board/boardlist.hpp"
#include "../../src/board/tile/rail/signal/signal3aspectrailtile.hpp"
#include "../../src/board/tile/rail/turnout/turnoutleft45railtile.hpp"
#include "../../src/board/tile/rail/turnout/turnout3wayrailtile.hpp"

TEST_CASE("OutputMap: Channel: Accessory -> ECoSObject -> Accessory", "[outputmap]")
{
  // Create world:
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE(worldWeak.lock()->interfaces->length == 0);
  REQUIRE(worldWeak.lock()->boards->length == 0);

  // Create interface:
  std::weak_ptr<ECoSInterface> interfaceWeak = std::dynamic_pointer_cast<ECoSInterface>(world->interfaces->create(ECoSInterface::classId));
  REQUIRE_FALSE(interfaceWeak.expired());
  REQUIRE(worldWeak.lock()->interfaces->length == 1);

  // Create board:
  std::weak_ptr<Board> boardWeak = world->boards->create();
  REQUIRE_FALSE(boardWeak.expired());
  REQUIRE(worldWeak.lock()->boards->length == 1);

  // Create signal:
  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg0, Signal3AspectRailTile::classId, false));
  std::weak_ptr<Signal3AspectRailTile> signalWeak = std::dynamic_pointer_cast<Signal3AspectRailTile>(boardWeak.lock()->getTile({0, 0}));
  std::weak_ptr<OutputMap> outputMapWeak = signalWeak.lock()->outputMap.value();
  REQUIRE_FALSE(signalWeak.expired());
  REQUIRE_FALSE(outputMapWeak.lock()->interface);

  // Assign interface:
  outputMapWeak.lock()->interface = interfaceWeak.lock();
  REQUIRE(outputMapWeak.lock()->interface.value() == interfaceWeak.lock());
  REQUIRE(isAccessory(outputMapWeak.lock()->channel.value()));
  REQUIRE(outputMapWeak.lock()->addresses.size() == 1);
  REQUIRE(outputMapWeak.lock()->ecosObject == 0);
  for(const auto& item : outputMapWeak.lock()->items)
  {
    REQUIRE(item->outputActions.size() == 1);
    REQUIRE(std::dynamic_pointer_cast<OutputMapPairOutputAction>(item->outputActions[0]));
  }

  // Add address:
  outputMapWeak.lock()->addAddress();
  REQUIRE(outputMapWeak.lock()->addresses.size() == 2);
  REQUIRE(outputMapWeak.lock()->addresses[0] != outputMapWeak.lock()->addresses[1]);
  for(const auto& item : outputMapWeak.lock()->items)
  {
    REQUIRE(item->outputActions.size() == 2);
    REQUIRE(std::dynamic_pointer_cast<OutputMapPairOutputAction>(item->outputActions[0]));
    REQUIRE(std::dynamic_pointer_cast<OutputMapPairOutputAction>(item->outputActions[1]));
  }

  // Change channel to ECoSObject:
  outputMapWeak.lock()->channel = OutputChannel::ECoSObject;
  REQUIRE(outputMapWeak.lock()->channel.value() == OutputChannel::ECoSObject);
  REQUIRE(outputMapWeak.lock()->addresses.size() == 0);
  REQUIRE(outputMapWeak.lock()->ecosObject == 0);
  for(const auto& item : outputMapWeak.lock()->items)
  {
    REQUIRE(item->outputActions.empty());
  }

  // Select ECoS object:
  outputMapWeak.lock()->ecosObject = 20000;
  REQUIRE(outputMapWeak.lock()->ecosObject == 20000);
  for(const auto& item : outputMapWeak.lock()->items)
  {
    REQUIRE(item->outputActions.size() == 1);
    REQUIRE(std::dynamic_pointer_cast<OutputMapECoSStateOutputAction>(item->outputActions[0]));
  }

  // Change channel to AccessoryMotorola:
  outputMapWeak.lock()->channel = OutputChannel::AccessoryMotorola;
  REQUIRE(outputMapWeak.lock()->channel.value() == OutputChannel::AccessoryMotorola);
  REQUIRE(outputMapWeak.lock()->addresses.size() == 1);
  REQUIRE(outputMapWeak.lock()->ecosObject == 0);
  for(const auto& item : outputMapWeak.lock()->items)
  {
    REQUIRE(item->outputActions.size() == 1);
    REQUIRE(std::dynamic_pointer_cast<OutputMapPairOutputAction>(item->outputActions[0]));
  }

  // cleanup:
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(interfaceWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(signalWeak.expired());
  REQUIRE(outputMapWeak.expired());
}

TEST_CASE("OutputMap: Channel: preserve mapping", "[outputmap]")
{
  // Create world:
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE(worldWeak.lock()->interfaces->length == 0);
  REQUIRE(worldWeak.lock()->boards->length == 0);

  // Create interface:
  std::weak_ptr<ECoSInterface> interfaceWeak = std::dynamic_pointer_cast<ECoSInterface>(world->interfaces->create(ECoSInterface::classId));
  REQUIRE_FALSE(interfaceWeak.expired());
  REQUIRE(worldWeak.lock()->interfaces->length == 1);

  // Create board:
  std::weak_ptr<Board> boardWeak = world->boards->create();
  REQUIRE_FALSE(boardWeak.expired());
  REQUIRE(worldWeak.lock()->boards->length == 1);

  // Create turnout:
  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg0, TurnoutLeft45RailTile::classId, false));
  std::weak_ptr<TurnoutLeft45RailTile> turnoutWeak = std::dynamic_pointer_cast<TurnoutLeft45RailTile>(boardWeak.lock()->getTile({0, 0}));
  std::weak_ptr<OutputMap> outputMapWeak = turnoutWeak.lock()->outputMap.value();
  REQUIRE_FALSE(turnoutWeak.expired());
  REQUIRE_FALSE(outputMapWeak.lock()->interface);

  // Assign interface:
  outputMapWeak.lock()->interface = interfaceWeak.lock();
  REQUIRE(outputMapWeak.lock()->interface.value() == interfaceWeak.lock());
  REQUIRE(outputMapWeak.lock()->channel.value() == OutputChannel::AccessoryDCC);
  REQUIRE(outputMapWeak.lock()->addresses.size() == 1);
  REQUIRE(outputMapWeak.lock()->ecosObject == 0);
  for(const auto& item : outputMapWeak.lock()->items)
  {
    REQUIRE(item->outputActions.size() == 1);
    REQUIRE(std::dynamic_pointer_cast<OutputMapPairOutputAction>(item->outputActions[0]));
  }

  // Assign output actions:
  std::weak_ptr<OutputMapPairOutputAction> outputActionWeakA = std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[0]->outputActions[0]);
  std::weak_ptr<OutputMapPairOutputAction> outputActionWeakB = std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[1]->outputActions[0]);
  outputActionWeakA.lock()->action = PairOutputAction::First;
  outputActionWeakB.lock()->action = PairOutputAction::Second;
  REQUIRE(outputActionWeakA.lock()->action.value() == PairOutputAction::First);
  REQUIRE(outputActionWeakB.lock()->action.value() == PairOutputAction::Second);

  // Change channel to AccessoryMotorola:
  outputMapWeak.lock()->channel = OutputChannel::AccessoryMotorola;
  REQUIRE(outputMapWeak.lock()->channel.value() == OutputChannel::AccessoryMotorola);
  REQUIRE_FALSE(outputActionWeakA.expired());
  REQUIRE_FALSE(outputActionWeakB.expired());
  REQUIRE(outputActionWeakA.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[0]->outputActions[0]));
  REQUIRE(outputActionWeakB.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[1]->outputActions[0]));
  REQUIRE(outputActionWeakA.lock()->action.value() == PairOutputAction::First);
  REQUIRE(outputActionWeakB.lock()->action.value() == PairOutputAction::Second);
  REQUIRE(outputMapWeak.lock()->addresses.size() == 1);
  REQUIRE(outputMapWeak.lock()->addresses[0] == 1);
  REQUIRE(outputMapWeak.lock()->ecosObject == 0);
  for(const auto& item : outputMapWeak.lock()->items)
  {
    REQUIRE(item->outputActions.size() == 1);
    REQUIRE(std::dynamic_pointer_cast<OutputMapPairOutputAction>(item->outputActions[0]));
  }

  // cleanup:
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(interfaceWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(turnoutWeak.expired());
  REQUIRE(outputMapWeak.expired());
}

TEST_CASE("OutputMap: Change interface, preserve mapping", "[outputmap]")
{
  // Create world:
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE(worldWeak.lock()->interfaces->length == 0);
  REQUIRE(worldWeak.lock()->boards->length == 0);

  // Create interfaces:
  std::weak_ptr<LocoNetInterface> loconetWeak = std::dynamic_pointer_cast<LocoNetInterface>(world->interfaces->create(LocoNetInterface::classId));
  std::weak_ptr<XpressNetInterface> xpressnetWeak = std::dynamic_pointer_cast<XpressNetInterface>(world->interfaces->create(XpressNetInterface::classId));
  std::weak_ptr<ECoSInterface> ecosWeak = std::dynamic_pointer_cast<ECoSInterface>(world->interfaces->create(ECoSInterface::classId));
  REQUIRE_FALSE(loconetWeak.expired());
  REQUIRE_FALSE(xpressnetWeak.expired());
  REQUIRE_FALSE(ecosWeak.expired());
  REQUIRE(worldWeak.lock()->interfaces->length == 3);

  // Create board:
  std::weak_ptr<Board> boardWeak = world->boards->create();
  REQUIRE_FALSE(boardWeak.expired());
  REQUIRE(worldWeak.lock()->boards->length == 1);

  // Create turnout:
  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg0, Turnout3WayRailTile::classId, false));
  std::weak_ptr<Turnout3WayRailTile> turnoutWeak = std::dynamic_pointer_cast<Turnout3WayRailTile>(boardWeak.lock()->getTile({0, 0}));
  std::weak_ptr<OutputMap> outputMapWeak = turnoutWeak.lock()->outputMap.value();
  REQUIRE_FALSE(turnoutWeak.expired());
  REQUIRE_FALSE(outputMapWeak.lock()->interface);

  // Assign interface:
  outputMapWeak.lock()->interface = loconetWeak.lock();
  REQUIRE(outputMapWeak.lock()->interface.value() == loconetWeak.lock());
  REQUIRE(isAccessory(outputMapWeak.lock()->channel.value()));
  REQUIRE(outputMapWeak.lock()->addresses.size() == 1);
  REQUIRE(outputMapWeak.lock()->ecosObject == 0);
  for(const auto& item : outputMapWeak.lock()->items)
  {
    REQUIRE(item->outputActions.size() == 1);
    REQUIRE(std::dynamic_pointer_cast<OutputMapPairOutputAction>(item->outputActions[0]));
  }

  // Change address:
  outputMapWeak.lock()->addresses.setInt64(0, 42);
  REQUIRE(outputMapWeak.lock()->addresses[0] == 42);

  // Add address:
  outputMapWeak.lock()->addAddress();
  REQUIRE(outputMapWeak.lock()->addresses.size() == 2);
  REQUIRE(outputMapWeak.lock()->addresses[1] == 43);
  for(const auto& item : outputMapWeak.lock()->items)
  {
    REQUIRE(item->outputActions.size() == 2);
    REQUIRE(std::dynamic_pointer_cast<OutputMapPairOutputAction>(item->outputActions[0]));
    REQUIRE(std::dynamic_pointer_cast<OutputMapPairOutputAction>(item->outputActions[1]));
  }

  // Assign output actions:
  std::weak_ptr<OutputMapPairOutputAction> outputActionWeakA0 = std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[0]->outputActions[0]);
  std::weak_ptr<OutputMapPairOutputAction> outputActionWeakA1 = std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[0]->outputActions[1]);
  std::weak_ptr<OutputMapPairOutputAction> outputActionWeakB0 = std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[1]->outputActions[0]);
  std::weak_ptr<OutputMapPairOutputAction> outputActionWeakB1 = std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[1]->outputActions[1]);
  std::weak_ptr<OutputMapPairOutputAction> outputActionWeakC0 = std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[2]->outputActions[0]);
  std::weak_ptr<OutputMapPairOutputAction> outputActionWeakC1 = std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[2]->outputActions[1]);
  outputActionWeakA0.lock()->action = PairOutputAction::First;
  outputActionWeakB0.lock()->action = PairOutputAction::Second;
  outputActionWeakB1.lock()->action = PairOutputAction::First;
  outputActionWeakC0.lock()->action = PairOutputAction::Second;
  outputActionWeakC1.lock()->action = PairOutputAction::Second;
  REQUIRE(outputActionWeakA0.lock()->action.value() == PairOutputAction::First);
  REQUIRE(outputActionWeakA1.lock()->action.value() == PairOutputAction::None);
  REQUIRE(outputActionWeakB0.lock()->action.value() == PairOutputAction::Second);
  REQUIRE(outputActionWeakB1.lock()->action.value() == PairOutputAction::First);
  REQUIRE(outputActionWeakC0.lock()->action.value() == PairOutputAction::Second);
  REQUIRE(outputActionWeakC1.lock()->action.value() == PairOutputAction::Second);

  // Change interface:
  outputMapWeak.lock()->interface = xpressnetWeak.lock();
  REQUIRE(outputMapWeak.lock()->interface.value() == xpressnetWeak.lock());
  REQUIRE(isAccessory(outputMapWeak.lock()->channel.value()));
  REQUIRE(outputMapWeak.lock()->addresses.size() == 2);
  REQUIRE(outputMapWeak.lock()->addresses[0] == 42);
  REQUIRE(outputMapWeak.lock()->addresses[1] == 43);
  for(const auto& item : outputMapWeak.lock()->items)
  {
    REQUIRE(item->outputActions.size() == 2);
    REQUIRE(std::dynamic_pointer_cast<OutputMapPairOutputAction>(item->outputActions[0]));
    REQUIRE(std::dynamic_pointer_cast<OutputMapPairOutputAction>(item->outputActions[1]));
  }
  REQUIRE(outputActionWeakA0.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[0]->outputActions[0]));
  REQUIRE(outputActionWeakA1.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[0]->outputActions[1]));
  REQUIRE(outputActionWeakB0.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[1]->outputActions[0]));
  REQUIRE(outputActionWeakB1.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[1]->outputActions[1]));
  REQUIRE(outputActionWeakC0.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[2]->outputActions[0]));
  REQUIRE(outputActionWeakC1.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[2]->outputActions[1]));
  REQUIRE(outputActionWeakA0.lock()->action.value() == PairOutputAction::First);
  REQUIRE(outputActionWeakA1.lock()->action.value() == PairOutputAction::None);
  REQUIRE(outputActionWeakB0.lock()->action.value() == PairOutputAction::Second);
  REQUIRE(outputActionWeakB1.lock()->action.value() == PairOutputAction::First);
  REQUIRE(outputActionWeakC0.lock()->action.value() == PairOutputAction::Second);
  REQUIRE(outputActionWeakC1.lock()->action.value() == PairOutputAction::Second);

  // Change interface again:
  outputMapWeak.lock()->interface = ecosWeak.lock();
  REQUIRE(outputMapWeak.lock()->interface.value() == ecosWeak.lock());
  REQUIRE(isAccessory(outputMapWeak.lock()->channel.value()));
  REQUIRE(outputMapWeak.lock()->addresses.size() == 2);
  REQUIRE(outputMapWeak.lock()->addresses[0] == 42);
  REQUIRE(outputMapWeak.lock()->addresses[1] == 43);
  for(const auto& item : outputMapWeak.lock()->items)
  {
    REQUIRE(item->outputActions.size() == 2);
    REQUIRE(std::dynamic_pointer_cast<OutputMapPairOutputAction>(item->outputActions[0]));
    REQUIRE(std::dynamic_pointer_cast<OutputMapPairOutputAction>(item->outputActions[1]));
  }
  REQUIRE(outputActionWeakA0.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[0]->outputActions[0]));
  REQUIRE(outputActionWeakA1.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[0]->outputActions[1]));
  REQUIRE(outputActionWeakB0.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[1]->outputActions[0]));
  REQUIRE(outputActionWeakB1.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[1]->outputActions[1]));
  REQUIRE(outputActionWeakC0.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[2]->outputActions[0]));
  REQUIRE(outputActionWeakC1.lock() == std::dynamic_pointer_cast<OutputMapPairOutputAction>(outputMapWeak.lock()->items[2]->outputActions[1]));
  REQUIRE(outputActionWeakA0.lock()->action.value() == PairOutputAction::First);
  REQUIRE(outputActionWeakA1.lock()->action.value() == PairOutputAction::None);
  REQUIRE(outputActionWeakB0.lock()->action.value() == PairOutputAction::Second);
  REQUIRE(outputActionWeakB1.lock()->action.value() == PairOutputAction::First);
  REQUIRE(outputActionWeakC0.lock()->action.value() == PairOutputAction::Second);
  REQUIRE(outputActionWeakC1.lock()->action.value() == PairOutputAction::Second);

  // cleanup:
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(loconetWeak.expired());
  REQUIRE(xpressnetWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(turnoutWeak.expired());
  REQUIRE(outputMapWeak.expired());
}
