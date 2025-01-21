/**
 * server/src/train/traintracking.cpp
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

#include "traintracking.hpp"
#include "train.hpp"
#include "trainblockstatus.hpp"
#include "../board/tile/rail/blockrailtile.hpp"
#include "../board/map/blockpath.hpp"
#include "../core/objectproperty.tpp"
#include "../world/world.hpp"

void TrainTracking::reserve(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block, BlockTrainDirection direction)
{
  block->trains.appendInternal(TrainBlockStatus::create(*block, *train, direction));
  block->updateTrainMethodEnabled();

  train->fireBlockReserved(block, direction);
  block->fireTrainReserved(train, direction);
}

void TrainTracking::enter(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block, BlockTrainDirection direction)
{
  auto status = TrainBlockStatus::create(*block, *train, direction);
  block->trains.appendInternal(status);

  block->updateTrainMethodEnabled();

  enter(status);
}

void TrainTracking::enter(const std::shared_ptr<TrainBlockStatus>& status)
{
  assert(status);
  const auto& train = status->train.value();
  const auto& block = status->block.value();
  const auto direction = status->direction.value();

  status->train->blocks.insertInternal(0, status); // head of train

  train->fireBlockEntered(block, direction);
  block->fireTrainEntered(train, direction);

  // Release any reserved tail blocks:
  // - When there is an undetected section (e.g. turnouts) between blocks,
  //   the train may leave the block before entering the next one causeing
  //   previous block be reserved.
  while(train->blocks.size() > 1)
  {
    const auto& tail = train->blocks.back();
    if(tail->block->state.value() == BlockState::Reserved)
    {
      left(tail);
    }
    else
    {
      break;
    }
  }
}

void TrainTracking::left(std::shared_ptr<TrainBlockStatus> status)
{
  const auto& train = status->train.value();
  const auto& block = status->block.value();
  const auto direction = status->direction.value();

  train->blocks.removeInternal(status);
  block->trains.removeInternal(status);

  block->updateTrainMethodEnabled();
  block->updateState();

  train->fireBlockLeft(block, direction);
  block->fireTrainLeft(train, direction);

  const auto pathA = block->getReservedPath(BlockSide::A);
  const bool exitA = pathA && &pathA->fromBlock() == block.get();

  const auto pathB = block->getReservedPath(BlockSide::B);
  const bool exitB = pathB && &pathB->fromBlock() == block.get();

  if(direction == BlockTrainDirection::TowardsA && exitA)
  {
    pathA->delayedRelease(block->world().pathReleaseDelay);
  }
  else if(direction == BlockTrainDirection::TowardsB && exitB)
  {
    pathB->delayedRelease(block->world().pathReleaseDelay);
  }

  status->destroy();
#ifndef NDEBUG
  std::weak_ptr<TrainBlockStatus> statusWeak = status;
  status.reset();
  assert(statusWeak.expired());
#endif
}
