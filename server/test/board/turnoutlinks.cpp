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
#include "../src/core/eventloop.hpp"
#include "../src/world/world.hpp"
#include "../src/core/method.tpp"
#include "../src/core/objectproperty.tpp"
#include "../src/board/board.hpp"
#include "../src/board/boardlist.hpp"
#include "../../src/board/tile/rail/blockrailtile.hpp"
#include "../../src/board/tile/rail/curve45railtile.hpp"
#include "../../src/board/tile/rail/turnout/turnoutleft45railtile.hpp"
#include "../../src/board/tile/rail/turnout/turnoutright45railtile.hpp"
#include "../../src/board/tile/rail/turnout/turnoutwyerailtile.hpp"

struct TurnoutLinkableRailTileTestAccess
{
  inline static auto convertPosition = TurnoutLinkableRailTile::convertPosition;
/*
  static TurnoutPosition convertPosition(const TurnoutLinkableRailTile& src, const TurnoutLinkableRailTile& dst, TurnoutPosition position, bool invert)
  {
    return TurnoutLinkableRailTile::convertPosition(src, dst, position, invert);
  }
*/
};

TEST_CASE("Board: Linked turnouts - convertPosition()", "[board]")
{
  EventLoop::reset();

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  std::weak_ptr<Board> boardWeak = world->boards->create();

  {
    auto convertPosition = TurnoutLinkableRailTileTestAccess::convertPosition;

    REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg0, TurnoutLeft45RailTile::classId, false));
    REQUIRE(boardWeak.lock()->addTile(2, 0, TileRotate::Deg0, TurnoutRight45RailTile::classId, false));
    REQUIRE(boardWeak.lock()->addTile(4, 0, TileRotate::Deg0, TurnoutWyeRailTile::classId, false));

    TurnoutLinkableRailTile& left = *boardWeak.lock()->getTile({0, 0})->shared_ptr<TurnoutLeft45RailTile>();
    TurnoutLinkableRailTile& right = *boardWeak.lock()->getTile({2, 0})->shared_ptr<TurnoutRight45RailTile>();
    TurnoutLinkableRailTile& wye = *boardWeak.lock()->getTile({4, 0})->shared_ptr<TurnoutWyeRailTile>();

    REQUIRE(convertPosition(left, left, TurnoutPosition::Unknown, false) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(left, left, TurnoutPosition::Straight, false) == TurnoutPosition::Straight);
    REQUIRE(convertPosition(left, left, TurnoutPosition::Left, false) == TurnoutPosition::Left);
    REQUIRE(convertPosition(left, left, TurnoutPosition::Unknown, true) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(left, left, TurnoutPosition::Straight, true) == TurnoutPosition::Left);
    REQUIRE(convertPosition(left, left, TurnoutPosition::Left, true) == TurnoutPosition::Straight);

    REQUIRE(convertPosition(right, right, TurnoutPosition::Unknown, false) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(right, right, TurnoutPosition::Straight, false) == TurnoutPosition::Straight);
    REQUIRE(convertPosition(right, right, TurnoutPosition::Right, false) == TurnoutPosition::Right);
    REQUIRE(convertPosition(right, right, TurnoutPosition::Unknown, true) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(right, right, TurnoutPosition::Straight, true) == TurnoutPosition::Right);
    REQUIRE(convertPosition(right, right, TurnoutPosition::Right, true) == TurnoutPosition::Straight);

    REQUIRE(convertPosition(wye, wye, TurnoutPosition::Unknown, false) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(wye, wye, TurnoutPosition::Right, false) == TurnoutPosition::Right);
    REQUIRE(convertPosition(wye, wye, TurnoutPosition::Left, false) == TurnoutPosition::Left);
    REQUIRE(convertPosition(wye, wye, TurnoutPosition::Unknown, true) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(wye, wye, TurnoutPosition::Right, true) == TurnoutPosition::Left);
    REQUIRE(convertPosition(wye, wye, TurnoutPosition::Left, true) == TurnoutPosition::Right);

    REQUIRE(convertPosition(left, right, TurnoutPosition::Unknown, false) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(left, right, TurnoutPosition::Straight, false) == TurnoutPosition::Straight);
    REQUIRE(convertPosition(left, right, TurnoutPosition::Left, false) == TurnoutPosition::Right);
    REQUIRE(convertPosition(left, right, TurnoutPosition::Unknown, true) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(left, right, TurnoutPosition::Straight, true) == TurnoutPosition::Right);
    REQUIRE(convertPosition(left, right, TurnoutPosition::Left, true) == TurnoutPosition::Straight);

    REQUIRE(convertPosition(left, wye, TurnoutPosition::Unknown, false) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(left, wye, TurnoutPosition::Straight, false) == TurnoutPosition::Right);
    REQUIRE(convertPosition(left, wye, TurnoutPosition::Left, false) == TurnoutPosition::Left);
    REQUIRE(convertPosition(left, wye, TurnoutPosition::Unknown, true) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(left, wye, TurnoutPosition::Straight, true) == TurnoutPosition::Left);
    REQUIRE(convertPosition(left, wye, TurnoutPosition::Left, true) == TurnoutPosition::Right);

    REQUIRE(convertPosition(right, left, TurnoutPosition::Unknown, false) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(right, left, TurnoutPosition::Straight, false) == TurnoutPosition::Straight);
    REQUIRE(convertPosition(right, left, TurnoutPosition::Right, false) == TurnoutPosition::Left);
    REQUIRE(convertPosition(right, left, TurnoutPosition::Unknown, true) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(right, left, TurnoutPosition::Straight, true) == TurnoutPosition::Left);
    REQUIRE(convertPosition(right, left, TurnoutPosition::Right, true) == TurnoutPosition::Straight);

    REQUIRE(convertPosition(right, wye, TurnoutPosition::Unknown, false) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(right, wye, TurnoutPosition::Straight, false) == TurnoutPosition::Left);
    REQUIRE(convertPosition(right, wye, TurnoutPosition::Right, false) == TurnoutPosition::Right);
    REQUIRE(convertPosition(right, wye, TurnoutPosition::Unknown, true) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(right, wye, TurnoutPosition::Straight, true) == TurnoutPosition::Right);
    REQUIRE(convertPosition(right, wye, TurnoutPosition::Right, true) == TurnoutPosition::Left);

    REQUIRE(convertPosition(wye, left, TurnoutPosition::Unknown, false) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(wye, left, TurnoutPosition::Right, false) == TurnoutPosition::Straight);
    REQUIRE(convertPosition(wye, left, TurnoutPosition::Left, false) == TurnoutPosition::Left);
    REQUIRE(convertPosition(wye, left, TurnoutPosition::Unknown, true) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(wye, left, TurnoutPosition::Right, true) == TurnoutPosition::Left);
    REQUIRE(convertPosition(wye, left, TurnoutPosition::Left, true) == TurnoutPosition::Straight);

    REQUIRE(convertPosition(wye, right, TurnoutPosition::Unknown, false) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(wye, right, TurnoutPosition::Right, false) == TurnoutPosition::Right);
    REQUIRE(convertPosition(wye, right, TurnoutPosition::Left, false) == TurnoutPosition::Straight);
    REQUIRE(convertPosition(wye, right, TurnoutPosition::Unknown, true) == TurnoutPosition::Unknown);
    REQUIRE(convertPosition(wye, right, TurnoutPosition::Right, true) == TurnoutPosition::Straight);
    REQUIRE(convertPosition(wye, right, TurnoutPosition::Left, true) == TurnoutPosition::Right);
  }

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}

TEST_CASE("Board: Linked turnouts - Double crossover", "[board]")
{
  EventLoop::reset();

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  // Board:
  // -\--/-
  // 1 \/ 2
  // 3 /\ 4
  // -/--\-
  std::weak_ptr<Board> boardWeak = world->boards->create();

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, TurnoutRight45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 0, TileRotate::Deg270, TurnoutLeft45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(0, 1, TileRotate::Deg90, TurnoutLeft45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 1, TileRotate::Deg270, TurnoutRight45RailTile::classId, false));

  auto turnoutWeak1 = boardWeak.lock()->getTile({0, 0})->weak_ptr<TurnoutRight45RailTile>();
  auto turnoutWeak2 = boardWeak.lock()->getTile({1, 0})->weak_ptr<TurnoutLeft45RailTile>();
  auto turnoutWeak3 = boardWeak.lock()->getTile({0, 1})->weak_ptr<TurnoutLeft45RailTile>();
  auto turnoutWeak4 = boardWeak.lock()->getTile({1, 1})->weak_ptr<TurnoutRight45RailTile>();

  REQUIRE_FALSE(turnoutWeak1.expired());
  REQUIRE_FALSE(turnoutWeak2.expired());
  REQUIRE_FALSE(turnoutWeak3.expired());
  REQUIRE_FALSE(turnoutWeak4.expired());

  world->run(); // this will build the board network

  REQUIRE(boardWeak.lock()->railCrossOver().find({0, 0}) != boardWeak.lock()->railCrossOver().end());
  std::weak_ptr<HiddenCrossOverRailTile> crossoverWeak = boardWeak.lock()->railCrossOver().find({0, 0})->second;
  REQUIRE_FALSE(crossoverWeak.expired());

  world->stop();

  REQUIRE(turnoutWeak1.lock()->position.value() == TurnoutPosition::Unknown);
  REQUIRE(turnoutWeak2.lock()->position.value() == TurnoutPosition::Unknown);
  REQUIRE(turnoutWeak3.lock()->position.value() == TurnoutPosition::Unknown);
  REQUIRE(turnoutWeak4.lock()->position.value() == TurnoutPosition::Unknown);

  turnoutWeak2.lock()->setPosition(TurnoutPosition::Left);

  REQUIRE(turnoutWeak1.lock()->position.value() == TurnoutPosition::Unknown);
  REQUIRE(turnoutWeak2.lock()->position.value() == TurnoutPosition::Left);
  REQUIRE(turnoutWeak3.lock()->position.value() == TurnoutPosition::Unknown);
  REQUIRE(turnoutWeak4.lock()->position.value() == TurnoutPosition::Unknown);

  turnoutWeak2.lock()->linked = true;
  turnoutWeak2.lock()->linkTurnout = turnoutWeak1.lock();

  REQUIRE(turnoutWeak1.lock()->position.value() == TurnoutPosition::Unknown);
  REQUIRE(turnoutWeak2.lock()->position.value() == TurnoutPosition::Unknown); // same as turnout1 due to link
  REQUIRE(turnoutWeak3.lock()->position.value() == TurnoutPosition::Unknown);
  REQUIRE(turnoutWeak4.lock()->position.value() == TurnoutPosition::Unknown);

  turnoutWeak2.lock()->setPosition(TurnoutPosition::Straight);

  REQUIRE(turnoutWeak1.lock()->position.value() == TurnoutPosition::Straight);
  REQUIRE(turnoutWeak2.lock()->position.value() == TurnoutPosition::Straight);
  REQUIRE(turnoutWeak3.lock()->position.value() == TurnoutPosition::Unknown);
  REQUIRE(turnoutWeak4.lock()->position.value() == TurnoutPosition::Unknown);

  turnoutWeak3.lock()->linked = true;
  turnoutWeak3.lock()->linkTurnout = turnoutWeak1.lock();

  REQUIRE(turnoutWeak1.lock()->position.value() == TurnoutPosition::Straight);
  REQUIRE(turnoutWeak2.lock()->position.value() == TurnoutPosition::Straight);
  REQUIRE(turnoutWeak3.lock()->position.value() == TurnoutPosition::Straight); // same as turnout1 due to created link
  REQUIRE(turnoutWeak4.lock()->position.value() == TurnoutPosition::Unknown);

  turnoutWeak1.lock()->setPosition(TurnoutPosition::Right);

  REQUIRE(turnoutWeak1.lock()->position.value() == TurnoutPosition::Right);
  REQUIRE(turnoutWeak2.lock()->position.value() == TurnoutPosition::Left);
  REQUIRE(turnoutWeak3.lock()->position.value() == TurnoutPosition::Left);
  REQUIRE(turnoutWeak4.lock()->position.value() == TurnoutPosition::Unknown);

  turnoutWeak4.lock()->linked = true;
  turnoutWeak4.lock()->linkTurnout = turnoutWeak1.lock();

  REQUIRE(turnoutWeak1.lock()->position.value() == TurnoutPosition::Right);
  REQUIRE(turnoutWeak2.lock()->position.value() == TurnoutPosition::Left);
  REQUIRE(turnoutWeak3.lock()->position.value() == TurnoutPosition::Left);
  REQUIRE(turnoutWeak4.lock()->position.value() == TurnoutPosition::Right);

  turnoutWeak4.lock()->setPosition(TurnoutPosition::Straight);

  REQUIRE(turnoutWeak1.lock()->position.value() == TurnoutPosition::Straight);
  REQUIRE(turnoutWeak2.lock()->position.value() == TurnoutPosition::Straight);
  REQUIRE(turnoutWeak3.lock()->position.value() == TurnoutPosition::Straight);
  REQUIRE(turnoutWeak4.lock()->position.value() == TurnoutPosition::Straight);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(crossoverWeak.expired());
  REQUIRE(turnoutWeak1.expired());
  REQUIRE(turnoutWeak2.expired());
  REQUIRE(turnoutWeak3.expired());
  REQUIRE(turnoutWeak4.expired());
}

TEST_CASE("Board: Linked turnouts - Crossover with invert", "[board]")
{
  EventLoop::reset();

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  // Board:
  // -\--
  //   \ 1
  // ---\ 2
  //     |
  std::weak_ptr<Board> boardWeak = world->boards->create();

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, TurnoutRight45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 1, TileRotate::Deg315, TurnoutLeft45RailTile::classId, false));

  auto turnoutWeak1 = boardWeak.lock()->getTile({0, 0})->weak_ptr<TurnoutRight45RailTile>();
  auto turnoutWeak2 = boardWeak.lock()->getTile({1, 1})->weak_ptr<TurnoutLeft45RailTile>();

  REQUIRE_FALSE(turnoutWeak1.expired());
  REQUIRE_FALSE(turnoutWeak2.expired());

  world->run(); // this will build the board network

  world->stop();

  REQUIRE(turnoutWeak1.lock()->position.value() == TurnoutPosition::Unknown);
  REQUIRE(turnoutWeak2.lock()->position.value() == TurnoutPosition::Unknown);

  turnoutWeak1.lock()->setPosition(TurnoutPosition::Straight);

  REQUIRE(turnoutWeak1.lock()->position.value() == TurnoutPosition::Straight);
  REQUIRE(turnoutWeak2.lock()->position.value() == TurnoutPosition::Unknown);

  turnoutWeak2.lock()->linked = true;
  turnoutWeak2.lock()->linkTurnout = turnoutWeak1.lock();

  REQUIRE(turnoutWeak1.lock()->position.value() == TurnoutPosition::Straight);
  REQUIRE(turnoutWeak2.lock()->position.value() == TurnoutPosition::Straight);

  turnoutWeak2.lock()->linkInvert = true;

  REQUIRE(turnoutWeak1.lock()->position.value() == TurnoutPosition::Straight);
  REQUIRE(turnoutWeak2.lock()->position.value() == TurnoutPosition::Left);

  turnoutWeak2.lock()->setPosition(TurnoutPosition::Straight);

  REQUIRE(turnoutWeak1.lock()->position.value() == TurnoutPosition::Right);
  REQUIRE(turnoutWeak2.lock()->position.value() == TurnoutPosition::Straight);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(turnoutWeak1.expired());
  REQUIRE(turnoutWeak2.expired());
}
