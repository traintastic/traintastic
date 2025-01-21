/**
 * server/test/worldlist.cpp
 *
 * This file is part of the traintastic test suite.
 *
 * Copyright (C) 2021 Reinder Feenstra
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
#include "../src/world/worldlist.hpp"

TEST_CASE("Create worldlist and model => destroy worldlist", "[worldlist]")
{
  const auto path = std::filesystem::temp_directory_path() / "traintastic-8emltd";
  {
    auto worldList = std::make_shared<WorldList>(path);
    REQUIRE(worldList);

    std::weak_ptr<WorldList> worldListWeak = worldList;
    REQUIRE_FALSE(worldListWeak.expired());

    auto model = worldList->getModel();
    REQUIRE(model);

    worldList.reset();
    REQUIRE_FALSE(worldListWeak.expired());

    model.reset();
    REQUIRE(worldListWeak.expired());
  }
  std::filesystem::remove_all(path);
}

TEST_CASE("Create worldlist and model => destroy model", "[worldlist]")
{
  const auto path = std::filesystem::temp_directory_path() / "traintastic-27oz9v";
  {
    auto worldList = std::make_shared<WorldList>(path);
    REQUIRE(worldList);

    std::weak_ptr<WorldList> worldListWeak = worldList;
    REQUIRE_FALSE(worldListWeak.expired());

    auto model = worldList->getModel();
    REQUIRE(model);

    std::weak_ptr<TableModel> modelWeak = model;
    REQUIRE_FALSE(modelWeak.expired());

    model.reset();
    REQUIRE(modelWeak.expired());

    worldList.reset();
    REQUIRE(worldListWeak.expired());
  }
  std::filesystem::remove_all(path);
}
