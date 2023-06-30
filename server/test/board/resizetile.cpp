/**
 * server/test/board/resizetile.cpp
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

#include <catch2/catch.hpp>
#include "../src/world/world.hpp"
#include "../src/core/method.tpp"
#include "../src/core/objectproperty.tpp"
#include "../src/board/board.hpp"
#include "../src/board/boardlist.hpp"
#include "../src/board/tile/rail/blockrailtile.hpp"

TEST_CASE("Board: Resize non existing tile", "[board][board-resize]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  auto board = world->boards->create();
  std::weak_ptr<Board> boardWeak = board;

  REQUIRE_FALSE(board->getTile({0, 0}));
  REQUIRE_FALSE(board->getTile({1, 1}));
  REQUIRE_FALSE(board->resizeTile(0, 0, 1, 1));
  REQUIRE_FALSE(board->getTile({0, 0}));
  REQUIRE_FALSE(board->getTile({1, 1}));

  board.reset();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}

TEST_CASE("Board: Resize block rail tile vertical", "[board][board-resize]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  auto board = world->boards->create();
  std::weak_ptr<Board> boardWeak = board;

  // add tile at 0,0
  REQUIRE(board->addTile(0, 0, TileRotate::Deg0, BlockRailTile::classId, false));
  std::weak_ptr<Tile> tile = board->getTile({0, 0});
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(tile.lock()->height == 1);
  REQUIRE(tile.lock()->width == 1);
  REQUIRE_FALSE(board->isTile({0, -1}));
  REQUIRE_FALSE(board->isTile({0, 1}));

  // resize to 1x6
  REQUIRE(board->resizeTile(0, 0, 1, 6));
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(tile.lock()->height == 6);
  REQUIRE(tile.lock()->width == 1);
  REQUIRE_FALSE(board->isTile({0, -1}));
  REQUIRE(board->getTile({0, 0}) == tile.lock());
  REQUIRE(board->getTile({0, 1}) == tile.lock());
  REQUIRE(board->getTile({0, 2}) == tile.lock());
  REQUIRE(board->getTile({0, 3}) == tile.lock());
  REQUIRE(board->getTile({0, 4}) == tile.lock());
  REQUIRE(board->getTile({0, 5}) == tile.lock());
  REQUIRE_FALSE(board->isTile({0, 6}));

  // resize to 1x3
  REQUIRE(board->resizeTile(0, 0, 1, 3));
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(tile.lock()->height == 3);
  REQUIRE(tile.lock()->width == 1);
  REQUIRE_FALSE(board->isTile({0, -1}));
  REQUIRE(board->getTile({0, 0}) == tile.lock());
  REQUIRE(board->getTile({0, 1}) == tile.lock());
  REQUIRE(board->getTile({0, 2}) == tile.lock());
  REQUIRE_FALSE(board->isTile({0, 3}));
  REQUIRE_FALSE(board->isTile({0, 4}));
  REQUIRE_FALSE(board->isTile({0, 5}));
  REQUIRE_FALSE(board->isTile({0, 6}));

  // resize to 2x2
  REQUIRE_FALSE(board->resizeTile(0, 0, 2, 2));
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(tile.lock()->height == 3);
  REQUIRE(tile.lock()->width == 1);
  REQUIRE_FALSE(board->isTile({0, -1}));
  REQUIRE(board->getTile({0, 0}) == tile.lock());
  REQUIRE(board->getTile({0, 1}) == tile.lock());
  REQUIRE(board->getTile({0, 2}) == tile.lock());
  REQUIRE_FALSE(board->isTile({0, 3}));
  REQUIRE_FALSE(board->isTile({0, 4}));
  REQUIRE_FALSE(board->isTile({0, 5}));
  REQUIRE_FALSE(board->isTile({0, 6}));

  // resize to 1x100
  REQUIRE_FALSE(board->resizeTile(0, 0, 1, 100));
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(tile.lock()->height == 3);
  REQUIRE(tile.lock()->width == 1);
  REQUIRE_FALSE(board->isTile({0, -1}));
  REQUIRE(board->getTile({0, 0}) == tile.lock());
  REQUIRE(board->getTile({0, 1}) == tile.lock());
  REQUIRE(board->getTile({0, 2}) == tile.lock());
  REQUIRE_FALSE(board->isTile({0, 3}));
  REQUIRE_FALSE(board->isTile({0, 4}));
  REQUIRE_FALSE(board->isTile({0, 5}));
  REQUIRE_FALSE(board->isTile({0, 6}));

  board.reset();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}

TEST_CASE("Board: Resize block rail tile horizontal", "[board][board-resize]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  auto board = world->boards->create();
  std::weak_ptr<Board> boardWeak = board;

  // add tile at 0,0
  REQUIRE(board->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  std::weak_ptr<Tile> tile = board->getTile({0, 0});
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(tile.lock()->height == 1);
  REQUIRE(tile.lock()->width == 1);
  REQUIRE_FALSE(board->isTile({-1, 0}));
  REQUIRE(board->getTile({0, 0}) == tile.lock());
  REQUIRE_FALSE(board->isTile({1, 0}));

  // resize to 6x1
  REQUIRE(board->resizeTile(0, 0, 6, 1));
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(tile.lock()->height == 1);
  REQUIRE(tile.lock()->width == 6);
  REQUIRE_FALSE(board->isTile({-1, 0}));
  REQUIRE(board->getTile({0, 0}) == tile.lock());
  REQUIRE(board->getTile({1, 0}) == tile.lock());
  REQUIRE(board->getTile({2, 0}) == tile.lock());
  REQUIRE(board->getTile({3, 0}) == tile.lock());
  REQUIRE(board->getTile({4, 0}) == tile.lock());
  REQUIRE(board->getTile({5, 0}) == tile.lock());
  REQUIRE_FALSE(board->isTile({6, 0}));

  // resize to 3x1
  REQUIRE(board->resizeTile(0, 0, 3, 1));
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(tile.lock()->height == 1);
  REQUIRE(tile.lock()->width == 3);
  REQUIRE_FALSE(board->isTile({-1, 0}));
  REQUIRE(board->getTile({0, 0}) == tile.lock());
  REQUIRE(board->getTile({1, 0}) == tile.lock());
  REQUIRE(board->getTile({2, 0}) == tile.lock());
  REQUIRE_FALSE(board->isTile({3, 0}));
  REQUIRE_FALSE(board->isTile({4, 0}));
  REQUIRE_FALSE(board->isTile({5, 0}));
  REQUIRE_FALSE(board->isTile({6, 0}));

  // resize to 2x2
  REQUIRE_FALSE(board->resizeTile(0, 0, 2, 2));
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(tile.lock()->height == 1);
  REQUIRE(tile.lock()->width == 3);
  REQUIRE_FALSE(board->isTile({-1, 0}));
  REQUIRE(board->getTile({0, 0}) == tile.lock());
  REQUIRE(board->getTile({1, 0}) == tile.lock());
  REQUIRE(board->getTile({2, 0}) == tile.lock());
  REQUIRE_FALSE(board->isTile({3, 0}));
  REQUIRE_FALSE(board->isTile({4, 0}));
  REQUIRE_FALSE(board->isTile({5, 0}));
  REQUIRE_FALSE(board->isTile({6, 0}));

  // resize to 100x1
  REQUIRE_FALSE(board->resizeTile(0, 0, 100, 1));
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(tile.lock()->height == 1);
  REQUIRE(tile.lock()->width == 3);
  REQUIRE_FALSE(board->isTile({-1, 0}));
  REQUIRE(board->getTile({0, 0}) == tile.lock());
  REQUIRE(board->getTile({1, 0}) == tile.lock());
  REQUIRE(board->getTile({2, 0}) == tile.lock());
  REQUIRE_FALSE(board->isTile({3, 0}));
  REQUIRE_FALSE(board->isTile({4, 0}));
  REQUIRE_FALSE(board->isTile({5, 0}));
  REQUIRE_FALSE(board->isTile({6, 0}));

  board.reset();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}
