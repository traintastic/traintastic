#include <catch2/catch_template_test_macros.hpp>
#include "../src/world/world.hpp"
#include "../src/core/method.tpp"
#include "../src/core/objectproperty.tpp"
#include "../src/board/board.hpp"
#include "../src/board/boardlist.hpp"
#include "../../src/board/tile/rail/blockrailtile.hpp"
#include "../../src/board/tile/rail/curve45railtile.hpp"
#include "../../src/board/tile/rail/turnout/turnoutleft45railtile.hpp"
#include "../../src/board/tile/rail/turnout/turnoutright45railtile.hpp"

TEST_CASE("Board: Crossover create/modify/destroy", "[board]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  // Board:
  // +--------+          +--------+
  // | block1 |---\  /---| block2 |
  // +--------+    \/    +--------+
  // +--------+    /\    +--------+
  // | block3 |---/--\---| block4 |
  // +--------+          +--------+
  std::weak_ptr<Board> boardWeak = world->boards->create();

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 0, TileRotate::Deg315, Curve45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(2, 0, TileRotate::Deg270, Curve45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(3, 0, TileRotate::Deg90, BlockRailTile::classId, false));

  REQUIRE(boardWeak.lock()->addTile(0, 1, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 1, TileRotate::Deg90, TurnoutLeft45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(2, 1, TileRotate::Deg270, TurnoutRight45RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(3, 1, TileRotate::Deg90, BlockRailTile::classId, false));

  world->run(); // this will build the board network

  REQUIRE(boardWeak.lock()->railCrossOver().find({1, 0}) != boardWeak.lock()->railCrossOver().end());
  std::weak_ptr<HiddenCrossOverRailTile> crossoverWeak = boardWeak.lock()->railCrossOver().find({1, 0})->second;

  world->stop();

  // modify the board, replace turnouts by curve:
  REQUIRE(boardWeak.lock()->addTile(1, 1, TileRotate::Deg90, Curve45RailTile::classId, true));
  REQUIRE(boardWeak.lock()->addTile(2, 1, TileRotate::Deg135, Curve45RailTile::classId, true));

  world->run(); // this will update the board network
  REQUIRE_FALSE(crossoverWeak.expired());

  world->stop();

  // remove one tile, so the crossover is no longer validp
  REQUIRE(boardWeak.lock()->deleteTile(2, 1));

  world->run(); // this will update the board network
  REQUIRE(crossoverWeak.expired()); // now the crossover must be gone

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
}
