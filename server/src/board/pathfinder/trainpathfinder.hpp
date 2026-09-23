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

#ifndef TRAINTASTIC_SERVER_BOARD_PATHFINDER_TRAINPATHFINDER_HPP
#define TRAINTASTIC_SERVER_BOARD_PATHFINDER_TRAINPATHFINDER_HPP

#include "pathfinder.hpp"
#include <optional>
#include "../../core/method.hpp"

class BlockPath;
class BlockRailTile;
enum class BlockSide : uint8_t;
enum class BlockTrainDirection : uint8_t;

class TrainPathFinder : public PathFinder
{
  CLASS_ID("path_finder.train")

public:
  Method<bool(const std::shared_ptr<BlockRailTile>&, BlockTrainDirection, const std::shared_ptr<BlockRailTile>&, BlockTrainDirection)> reserve;

  TrainPathFinder(Object& parent_, std::string_view parentPropertyName);

  static std::vector<BlockRailTile*> find(BlockRailTile& from, std::optional<BlockSide> fromSide, BlockRailTile& to, std::optional<BlockSide>& toSide);

private:
  static std::vector<BlockRailTile*> dijkstra(BlockRailTile& from, std::optional<BlockSide> fromSide, BlockRailTile& to, std::optional<BlockSide>& toSide, std::function<uint64_t(const BlockPath&, BlockSide)> getCost);
};

#endif
