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
#include "../src/world/world.hpp"
#include "../src/world/worldloader.hpp"
#include "../src/world/worldsaver.hpp"
#include "../src/core/method.tpp"
#include "../src/core/objectproperty.tpp"
#include "../src/hardware/decoder/list/decoderlist.hpp"
#include "../src/vehicle/rail/locomotive.hpp"
#include "../src/vehicle/rail/railvehiclelist.hpp"
#include "../src/train/train.hpp"
#include "../src/train/trainlist.hpp"
#include "../src/train/trainvehiclelist.hpp"

TEST_CASE("Train: Save/Load", "[train][train-saveload]")
{
  std::filesystem::path ctw;
  std::string worldUUID;

  {
    INFO("Create world");
    auto world = World::create();

    {
      worldUUID = world->uuid;

      auto decoder = world->decoders->create();

      auto locomotive = world->railVehicles->create(Locomotive::classId);
      locomotive->decoder = decoder;
      REQUIRE(decoder->vehicle.value() == locomotive);

      auto train = world->trains->create();
      train->vehicles->add(locomotive);
      REQUIRE(locomotive->trains[0] == train);
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

      REQUIRE(world->decoders->length == 1);
      auto decoder = world->decoders->operator[](0);
      REQUIRE(decoder);

      REQUIRE(world->railVehicles->length == 1);
      auto locomotive = std::dynamic_pointer_cast<Locomotive>(world->railVehicles->operator[](0));
      REQUIRE(locomotive);

      REQUIRE(world->trains->length == 1);
      auto train = world->trains->operator[](0);
      REQUIRE(train);

      REQUIRE(decoder->vehicle.value() == locomotive);

      REQUIRE(locomotive->decoder.value() == decoder);
      REQUIRE(locomotive->trains.size() == 1);
      REQUIRE(locomotive->trains[0] == train);

      REQUIRE(train->vehicles->length == 1);
      REQUIRE(train->vehicles->operator[](0) == locomotive);
    }
  }

  INFO("Remove saved world");
  REQUIRE(std::filesystem::remove(ctw));
}
