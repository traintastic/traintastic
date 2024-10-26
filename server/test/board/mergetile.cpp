/**
 * server/test/board/addtile.cpp
 *
 * This file is part of the traintastic test suite.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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
#include "../src/world/world.hpp"
#include "../src/core/method.tpp"
#include "../src/core/objectproperty.tpp"
#include "../src/board/board.hpp"
#include "../src/board/boardlist.hpp"
#include "../src/board/tile/rail/straightrailtile.hpp"
#include "../src/board/tile/rail/bridge90railtile.hpp"
#include "../src/board/tile/rail/bridge45leftrailtile.hpp"
#include "../src/board/tile/rail/bridge45rightrailtile.hpp"

TEST_CASE("Board: Merge tile, brigde 90", "[board][board-merge]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  auto board = world->boards->create();
  std::weak_ptr<Board> boardWeak = board;

  int16_t x = 0;
  const int16_t y = 0;
  for(auto r : {TileRotate::Deg0, TileRotate::Deg45, TileRotate::Deg90, TileRotate::Deg135})
  {
    REQUIRE(board->addTile(x, y, r, StraightRailTile::classId, false));
    std::weak_ptr<Tile> straight = board->getTile({x, y});
    REQUIRE_FALSE(straight.expired());
    REQUIRE(straight.lock()->rotate == r);

    REQUIRE(board->addTile(x, y, r + TileRotate::Deg90, StraightRailTile::classId, false));
    REQUIRE(straight.expired());
    std::weak_ptr<Tile> bridge = board->getTile({x, y});
    REQUIRE_FALSE(bridge.expired());
    REQUIRE(bridge.lock()->getClassId() == Bridge90RailTile::classId);
    REQUIRE(bridge.lock()->rotate == r);

    x++;
  }

  board.reset();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}

TEST_CASE("Board: Merge tile, brigde 45 left", "[board][board-merge]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  auto board = world->boards->create();
  std::weak_ptr<Board> boardWeak = board;

  int16_t x = 0;
  const int16_t y = 0;
  for(auto r : {TileRotate::Deg0, TileRotate::Deg45, TileRotate::Deg90, TileRotate::Deg135})
  {
    REQUIRE(board->addTile(x, y, r, StraightRailTile::classId, false));
    std::weak_ptr<Tile> straight = board->getTile({x, y});
    REQUIRE_FALSE(straight.expired());
    REQUIRE(straight.lock()->rotate == r);

    REQUIRE(board->addTile(x, y, r - TileRotate::Deg45, StraightRailTile::classId, false));
    REQUIRE(straight.expired());
    std::weak_ptr<Tile> bridge = board->getTile({x, y});
    REQUIRE_FALSE(bridge.expired());
    REQUIRE(bridge.lock()->getClassId() == Bridge45LeftRailTile::classId);
    REQUIRE(bridge.lock()->rotate == r);

    x++;
  }

  board.reset();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}

TEST_CASE("Board: Merge tile, brigde 45 right", "[board][board-merge]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  auto board = world->boards->create();
  std::weak_ptr<Board> boardWeak = board;

  int16_t x = 0;
  const int16_t y = 0;
  for(auto r : {TileRotate::Deg0, TileRotate::Deg45, TileRotate::Deg90, TileRotate::Deg135})
  {
    REQUIRE(board->addTile(x, y, r, StraightRailTile::classId, false));
    std::weak_ptr<Tile> straight = board->getTile({x, y});
    REQUIRE_FALSE(straight.expired());
    REQUIRE(straight.lock()->rotate == r);

    REQUIRE(board->addTile(x, y, r + TileRotate::Deg45, StraightRailTile::classId, false));
    REQUIRE(straight.expired());
    std::weak_ptr<Tile> bridge = board->getTile({x, y});
    REQUIRE_FALSE(bridge.expired());
    REQUIRE(bridge.lock()->getClassId() == Bridge45RightRailTile::classId);
    REQUIRE(bridge.lock()->rotate == r);

    x++;
  }

  board.reset();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}
