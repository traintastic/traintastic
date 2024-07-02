/**
 * server/src/train/traintracking.hpp
 *
 * This file is part of the traintastic source code.
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

#ifndef TRAINTASTIC_SERVER_TRAIN_TRAINTRACKING_HPP
#define TRAINTASTIC_SERVER_TRAIN_TRAINTRACKING_HPP

#include <memory>
#include <traintastic/enum/blocktraindirection.hpp>

class Train;
class BlockRailTile;
class TrainBlockStatus;

class TrainTracking final
{
private:
  TrainTracking() = default;
  TrainTracking(const TrainTracking&) = delete;
  TrainTracking& operator=(const TrainTracking&) = delete;

public:
  static void reserve(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block, BlockTrainDirection direction);
  static void enter(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block, BlockTrainDirection direction); // enter unreserved block
  static void enter(const std::shared_ptr<TrainBlockStatus>& status); // enter reserved block
  static void left(std::shared_ptr<TrainBlockStatus> status);
};

#endif
