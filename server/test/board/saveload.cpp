/**
 * server/test/board/saveload.cpp
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
#include "../src/world/worldloader.hpp"
#include "../src/world/worldsaver.hpp"
#include "../src/board/board.hpp"
#include "../src/board/tile/rail/curve90railtile.hpp"

TEST_CASE("Board: Save/Load", "[board][board-saveload]")
{
  std::filesystem::path ctw;
  std::string worldUUID;
  std::string boardId;

  {
    INFO("Create world");
    auto world = World::create();

    {
      worldUUID = world->uuid;
      auto board = world->boards->create();
      boardId = board->id;

      REQUIRE(board->addTile(0, 0, TileRotate::Deg270, Curve90RailTile::classId, false));
    }

    INFO("Saving...");
    {
      ctw = std::filesystem::temp_directory_path() / std::string(world->uuid.value()).append(World::dotCTW);
      WorldSaver saver(*world, ctw);
    }
    INFO("Saved");
  }

  {
    std::shared_ptr<World> world;
    {
      INFO("Loading...");
      WorldLoader loader(ctw);
      world = loader.world();
      REQUIRE(world);
      INFO("Loaded");
    }

    {
      REQUIRE(world->uuid.value() == worldUUID);
      REQUIRE(world->boards->length == 1);

      auto board = world->boards->operator[](0);
      REQUIRE(board);
      REQUIRE(board->id.value() == boardId);

      auto tile_0_0 = board->getTile({0, 0});
      REQUIRE(tile_0_0);
      REQUIRE(tile_0_0->getClassId() == Curve90RailTile::classId);
      REQUIRE(tile_0_0->rotate == TileRotate::Deg270);
    }
  }

  INFO("Remove saved world");
  REQUIRE(std::filesystem::remove(ctw));
}
