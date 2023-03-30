/**
 * server/test/board/movetile.cpp
 *
 * This file is part of the traintastic test suite.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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
#include "../src/core/objectproperty.tpp"
#include "../src/board/board.hpp"
#include "../src/board/boardlist.hpp"
#include "../src/board/tile/rail/straightrailtile.hpp"
#include "../src/board/tile/rail/blockrailtile.hpp"

TEST_CASE("Board: Move non existing tile", "[board][board-move]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  auto board = world->boards->create();
  std::weak_ptr<Board> boardWeak = board;

  REQUIRE_FALSE(board->getTile({0, 0}));
  REQUIRE_FALSE(board->getTile({1, 1}));
  REQUIRE_FALSE(board->moveTile(0, 0, 1, 1, TileRotate::Deg0, false));
  REQUIRE_FALSE(board->getTile({0, 0}));
  REQUIRE_FALSE(board->getTile({1, 1}));

  board.reset();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}

TEST_CASE("Board: Move 1x1 tile to empty location", "[board][board-move]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  auto board = world->boards->create();
  std::weak_ptr<Board> boardWeak = board;

  // add tile at 0,0
  REQUIRE(board->addTile(0, 0, TileRotate::Deg0, StraightRailTile::classId, false));
  std::weak_ptr<Tile> tile = board->getTile({0, 0});
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});

  // move without replace
  REQUIRE(board->moveTile(0, 0, 1, 1, TileRotate::Deg0, false));
  REQUIRE_FALSE(board->getTile({0, 0}));
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{1, 1});
  REQUIRE(board->getTile({1, 1}) == tile.lock());

  // move with replace
  REQUIRE(board->moveTile(1, 1, 2, 2, TileRotate::Deg0, true));
  REQUIRE_FALSE(board->getTile({1, 1}));
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{2, 2});
  REQUIRE(board->getTile({2, 2}) == tile.lock());

  board.reset();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}

TEST_CASE("Board: Move 1x1 tile to occupied location", "[board][board-move]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  auto board = world->boards->create();
  std::weak_ptr<Board> boardWeak = board;

  // add tile at 0,0
  REQUIRE(board->addTile(0, 0, TileRotate::Deg0, StraightRailTile::classId, false));
  std::weak_ptr<Tile> tile0 = board->getTile({0, 0});
  REQUIRE_FALSE(tile0.expired());
  REQUIRE(tile0.lock()->location() == TileLocation{0, 0});

  // add tile at 1,1
  REQUIRE(board->addTile(1, 1, TileRotate::Deg0, StraightRailTile::classId, false));
  std::weak_ptr<Tile> tile1 = board->getTile({1, 1});
  REQUIRE_FALSE(tile1.expired());
  REQUIRE(tile1.lock()->location() == TileLocation{1, 1});

  // move without replace
  REQUIRE_FALSE(board->moveTile(0, 0, 1, 1, TileRotate::Deg0, false));
  REQUIRE_FALSE(tile0.expired());
  REQUIRE(tile0.lock()->location() == TileLocation{0, 0});
  REQUIRE(board->getTile({0, 0}) == tile0.lock());
  REQUIRE_FALSE(tile1.expired());
  REQUIRE(tile1.lock()->location() == TileLocation{1, 1});
  REQUIRE(board->getTile({1, 1}) == tile1.lock());

  // move with replace
  REQUIRE(board->moveTile(0, 0, 1, 1, TileRotate::Deg0, true));
  REQUIRE_FALSE(tile0.expired());
  REQUIRE_FALSE(board->getTile({0, 0}));
  REQUIRE(tile0.lock()->location() == TileLocation{1, 1});
  REQUIRE(board->getTile({1, 1}) == tile0.lock());
  REQUIRE(tile1.expired());

  board.reset();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}

TEST_CASE("Board: Move 1x1 tile to invalid location", "[board][board-move]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  auto board = world->boards->create();
  std::weak_ptr<Board> boardWeak = board;

  // add tile at 0,0
  REQUIRE(board->addTile(0, 0, TileRotate::Deg0, StraightRailTile::classId, false));
  std::weak_ptr<Tile> tile = board->getTile({0, 0});
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});

  // move far top
  REQUIRE_FALSE(board->moveTile(0, 0, 0, Board::sizeMin - 1, TileRotate::Deg0, false));
  REQUIRE_FALSE(board->getTile({0, std::numeric_limits<int16_t>::min()}));
  REQUIRE(board->getTile({0, 0}));
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(board->getTile({0, 0}) == tile.lock());

  // move far bottom
  REQUIRE_FALSE(board->moveTile(0, 0, 0, Board::sizeMax + 1, TileRotate::Deg0, false));
  REQUIRE_FALSE(board->getTile({0, std::numeric_limits<int16_t>::max()}));
  REQUIRE(board->getTile({0, 0}));
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(board->getTile({0, 0}) == tile.lock());

  // move left
  REQUIRE_FALSE(board->moveTile(0, 0, Board::sizeMin - 1, 0, TileRotate::Deg0, false));
  REQUIRE_FALSE(board->getTile({std::numeric_limits<int16_t>::min(), 0}));
  REQUIRE(board->getTile({0, 0}));
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(board->getTile({0, 0}) == tile.lock());

  // move right
  REQUIRE_FALSE(board->moveTile(0, 0, Board::sizeMax + 1, 0, TileRotate::Deg0, false));
  REQUIRE_FALSE(board->getTile({std::numeric_limits<int16_t>::max(), 0}));
  REQUIRE(board->getTile({0, 0}));
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(board->getTile({0, 0}) == tile.lock());

  board.reset();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}

TEST_CASE("Board: Move 5x1 tile to occupied location", "[board][board-move]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  auto board = world->boards->create();
  std::weak_ptr<Board> boardWeak = board;

  // add tile at 0,0
  REQUIRE(board->addTile(0, 0, TileRotate::Deg0, BlockRailTile::classId, false));
  REQUIRE(board->resizeTile(0, 0, 1, 5));
  std::weak_ptr<Tile> tile0 = board->getTile({0, 0});
  REQUIRE_FALSE(tile0.expired());
  REQUIRE(tile0.lock()->location() == TileLocation{0, 0});
  REQUIRE(board->getTile({0, 1}) == tile0.lock());
  REQUIRE(board->getTile({0, 2}) == tile0.lock());
  REQUIRE(board->getTile({0, 3}) == tile0.lock());
  REQUIRE(board->getTile({0, 4}) == tile0.lock());

  // add tile at 1,1
  REQUIRE(board->addTile(1, 1, TileRotate::Deg0, BlockRailTile::classId, false));
  REQUIRE(board->resizeTile(1, 1, 1, 5));
  std::weak_ptr<Tile> tile1 = board->getTile({1, 1});
  REQUIRE_FALSE(tile1.expired());
  REQUIRE(tile1.lock()->location() == TileLocation{1, 1});
  REQUIRE(board->getTile({1, 2}) == tile1.lock());
  REQUIRE(board->getTile({1, 3}) == tile1.lock());
  REQUIRE(board->getTile({1, 4}) == tile1.lock());
  REQUIRE(board->getTile({1, 5}) == tile1.lock());

  // move without replace, partly replace other tile
  REQUIRE_FALSE(board->moveTile(0, 0, 1, 0, TileRotate::Deg0, false));
  REQUIRE_FALSE(board->isTile({1, 0}));
  REQUIRE_FALSE(tile0.expired());
  REQUIRE(tile0.lock()->location() == TileLocation{0, 0});
  REQUIRE(board->getTile({0, 0}) == tile0.lock());
  REQUIRE(board->getTile({0, 1}) == tile0.lock());
  REQUIRE(board->getTile({0, 2}) == tile0.lock());
  REQUIRE(board->getTile({0, 3}) == tile0.lock());
  REQUIRE(board->getTile({0, 4}) == tile0.lock());
  REQUIRE_FALSE(tile1.expired());
  REQUIRE(tile1.lock()->location() == TileLocation{1, 1});
  REQUIRE(board->getTile({1, 1}) == tile1.lock());
  REQUIRE(board->getTile({1, 2}) == tile1.lock());
  REQUIRE(board->getTile({1, 3}) == tile1.lock());
  REQUIRE(board->getTile({1, 4}) == tile1.lock());
  REQUIRE(board->getTile({1, 5}) == tile1.lock());

  // move with replace, partly replace other tile
  REQUIRE(board->moveTile(0, 0, 1, 0, TileRotate::Deg0, true));
  REQUIRE_FALSE(tile0.expired());
  REQUIRE_FALSE(board->isTile({0, 0}));
  REQUIRE_FALSE(board->isTile({0, 1}));
  REQUIRE_FALSE(board->isTile({0, 2}));
  REQUIRE_FALSE(board->isTile({0, 3}));
  REQUIRE_FALSE(board->isTile({0, 4}));
  REQUIRE(tile0.lock()->location() == TileLocation{1, 0});
  REQUIRE(board->getTile({1, 0}) == tile0.lock());
  REQUIRE(board->getTile({1, 1}) == tile0.lock());
  REQUIRE(board->getTile({1, 2}) == tile0.lock());
  REQUIRE(board->getTile({1, 3}) == tile0.lock());
  REQUIRE(board->getTile({1, 4}) == tile0.lock());
  REQUIRE_FALSE(board->getTile({0, 5}));
  REQUIRE(tile1.expired());

  board.reset();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}

TEST_CASE("Board: Move 5x1 tile replace itself partly", "[board][board-move]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  auto board = world->boards->create();
  std::weak_ptr<Board> boardWeak = board;

  // add tile at 0,0
  REQUIRE(board->addTile(0, 0, TileRotate::Deg0, BlockRailTile::classId, false));
  REQUIRE(board->resizeTile(0, 0, 1, 5));
  std::weak_ptr<Tile> tile = board->getTile({0, 0});
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(board->getTile({0, 1}) == tile.lock());
  REQUIRE(board->getTile({0, 2}) == tile.lock());
  REQUIRE(board->getTile({0, 3}) == tile.lock());
  REQUIRE(board->getTile({0, 4}) == tile.lock());

  // move without replace, partly replacing itself
  REQUIRE(board->moveTile(0, 0, 0, 2, TileRotate::Deg0, false));
  REQUIRE_FALSE(board->isTile({0, 0}));
  REQUIRE_FALSE(board->isTile({0, 1}));
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{0, 2});
  REQUIRE(tile.lock()->rotate.value() == TileRotate::Deg0);
  REQUIRE(tile.lock()->height.value() == 5);
  REQUIRE(tile.lock()->width.value() == 1);
  REQUIRE(board->getTile({0, 2}) == tile.lock());
  REQUIRE(board->getTile({0, 3}) == tile.lock());
  REQUIRE(board->getTile({0, 4}) == tile.lock());
  REQUIRE(board->getTile({0, 5}) == tile.lock());
  REQUIRE(board->getTile({0, 6}) == tile.lock());

  board.reset();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}

TEST_CASE("Board: Move and rotate 5x1 tile replace itself partly", "[board][board-move]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  auto board = world->boards->create();
  std::weak_ptr<Board> boardWeak = board;

  // add tile at 0,0
  REQUIRE(board->addTile(0, 0, TileRotate::Deg0, BlockRailTile::classId, false));
  REQUIRE(board->resizeTile(0, 0, 1, 5));
  std::weak_ptr<Tile> tile = board->getTile({0, 0});
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{0, 0});
  REQUIRE(board->getTile({0, 1}) == tile.lock());
  REQUIRE(board->getTile({0, 2}) == tile.lock());
  REQUIRE(board->getTile({0, 3}) == tile.lock());
  REQUIRE(board->getTile({0, 4}) == tile.lock());

  // move and rotate without replace, partly replacing itself
  REQUIRE(board->moveTile(0, 0, 0, 2, TileRotate::Deg90, false));
  REQUIRE_FALSE(board->isTile({0, 0}));
  REQUIRE_FALSE(board->isTile({0, 1}));
  REQUIRE_FALSE(board->isTile({0, 3}));
  REQUIRE_FALSE(board->isTile({0, 4}));
  REQUIRE_FALSE(tile.expired());
  REQUIRE(tile.lock()->location() == TileLocation{0, 2});
  REQUIRE(tile.lock()->rotate.value() == TileRotate::Deg90);
  REQUIRE(tile.lock()->height.value() == 1);
  REQUIRE(tile.lock()->width.value() == 5);
  REQUIRE(board->getTile({0, 2}) == tile.lock());
  REQUIRE(board->getTile({1, 2}) == tile.lock());
  REQUIRE(board->getTile({2, 2}) == tile.lock());
  REQUIRE(board->getTile({3, 2}) == tile.lock());
  REQUIRE(board->getTile({4, 2}) == tile.lock());

  board.reset();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}
