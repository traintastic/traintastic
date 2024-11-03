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
#include "../zone/blockzonelist.hpp"

void TrainTracking::assigned(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block)
{
  assert(train);
  assert(block);
  checkZoneAssigned(train, block);
  train->fireBlockAssigned(block);
  block->fireTrainAssigned(train);
}
#include "../world/world.hpp"

void TrainTracking::reserve(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block, BlockTrainDirection direction)
{
  assert(train);
  assert(block);

  checkZoneLeaving(train, block);

  block->trains.appendInternal(TrainBlockStatus::create(*block, *train, direction));
  block->updateTrainMethodEnabled();

  checkZoneEntering(train, block);

  train->fireBlockReserved(block, direction);
  block->fireTrainReserved(train, direction);
}

void TrainTracking::enter(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block, BlockTrainDirection direction)
{
  assert(train);
  assert(block);

  checkZoneLeaving(train, block);

  auto status = TrainBlockStatus::create(*block, *train, direction);
  block->trains.appendInternal(status);

  block->updateTrainMethodEnabled();

  enter(status);
}

void TrainTracking::enter(const std::shared_ptr<TrainBlockStatus>& blockStatus)
{
  assert(blockStatus);
  const auto& train = blockStatus->train.value();
  const auto& block = blockStatus->block.value();
  const auto direction = blockStatus->direction.value();

  blockStatus->train->blocks.insertInternal(0, blockStatus); // head of train

  checkZoneEntering(train, block);
  checkZoneEntered(train, block);

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

void TrainTracking::left(std::shared_ptr<TrainBlockStatus> blockStatus)
{
  assert(blockStatus);
  const auto& train = blockStatus->train.value();
  const auto& block = blockStatus->block.value();
  const auto direction = blockStatus->direction.value();

  train->blocks.removeInternal(blockStatus);
  block->trains.removeInternal(blockStatus);

  block->updateTrainMethodEnabled();
  block->updateState();

  checkZoneEntered(train, block);
  checkZoneLeft(train, block);

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

  blockStatus->destroy();
#ifndef NDEBUG
  std::weak_ptr<TrainBlockStatus> blockStatusWeak = blockStatus;
  blockStatus.reset();
  assert(blockStatusWeak.expired());
#endif
}

void TrainTracking::removed(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block)
{
  assert(train);
  assert(block);
  checkZoneRemoved(train, block);
  train->fireBlockRemoved(block);
  block->fireTrainRemoved(train);
}

void TrainTracking::checkZoneAssigned(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block)
{
  assert(train);
  assert(block);

  for(auto& zone : *block->zones)
  {
    auto zoneStatus = zone->getTrainZoneStatus(train);
    if(zoneStatus)
    {
      continue; // train already known
    }

    zoneStatus = TrainZoneStatus::create(*zone, *train, ZoneTrainState::Entered);
    zone->trains.appendInternal(zoneStatus);
    train->zones.appendInternal(zoneStatus);

    train->fireZoneAssigned(zone);
    zone->fireTrainAssigned(train);
  }
}

void TrainTracking::checkZoneEntering(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block)
{
  assert(train);
  assert(block);

  for(auto& zone : *block->zones)
  {
    auto zoneStatus = zone->getTrainZoneStatus(train);
    if(zoneStatus && (zoneStatus->state == ZoneTrainState::Entering || zoneStatus->state == ZoneTrainState::Entered))
    {
      continue; // train already in or entering zone
    }

    if(!zoneStatus)
    {
      zoneStatus = TrainZoneStatus::create(*zone, *train, ZoneTrainState::Entering);
      zone->trains.appendInternal(zoneStatus);
      train->zones.appendInternal(zoneStatus);
    }
    else
    {
      assert(zoneStatus->state.value() == ZoneTrainState::Leaving);
      zoneStatus->state.setValueInternal(ZoneTrainState::Entering);
    }

    zone->fireTrainEntering(train);
    train->fireZoneEntering(zone);
  }
}

void TrainTracking::checkZoneEntered(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block)
{
  assert(train);
  assert(block);

  for(auto& zone : *block->zones)
  {
    auto zoneStatus = zone->getTrainZoneStatus(train);
    if(zoneStatus && zoneStatus->state == ZoneTrainState::Entered)
    {
      continue; // train already in zone
    }

    if(!zoneStatus)
    {
      zoneStatus = TrainZoneStatus::create(*zone, *train, ZoneTrainState::Entered);
      zone->trains.appendInternal(zoneStatus);
      train->zones.appendInternal(zoneStatus);
    }
    else
    {
      assert(zoneStatus->state == ZoneTrainState::Entering || zoneStatus->state.value() == ZoneTrainState::Leaving);
      zoneStatus->state.setValueInternal(ZoneTrainState::Entered);
    }

    trainEnteredOrLeftZone(*train, *zone);

    zone->fireTrainEntered(train);
    train->fireZoneEntered(zone);
  }
}

void TrainTracking::checkZoneLeaving(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block)
{
  assert(train);
  assert(block);

  for(const auto& zoneStatus : *train->zones)
  {
    const auto& zone = zoneStatus->zone.value();
    if(!block->zones->containsObject(zone))
    {
      zoneStatus->state.setValueInternal(ZoneTrainState::Leaving);
      zone->fireTrainLeaving(train);
      train->fireZoneLeaving(zone);
    }
  }
}

void TrainTracking::checkZoneLeft(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block)
{
  assert(train);
  assert(block);

  for(const auto& zone : *block->zones)
  {
    if(removeTrainIfNotInZone(train, zone))
    {
      trainEnteredOrLeftZone(*train, *zone);

      zone->fireTrainLeft(train);
      train->fireZoneLeft(zone);
    }
  }
}

void TrainTracking::checkZoneRemoved(const std::shared_ptr<Train>& train, const std::shared_ptr<BlockRailTile>& block)
{
  assert(train);
  assert(block);

  for(auto& zone : *block->zones)
  {
    if(removeTrainIfNotInZone(train, zone))
    {
      train->fireZoneRemoved(zone);
      zone->fireTrainRemoved(train);
    }
  }
}

bool TrainTracking::removeTrainIfNotInZone(const std::shared_ptr<Train>& train, const std::shared_ptr<Zone>& zone)
{
  assert(train);
  assert(zone);

  if(std::find_if(train->blocks.begin(), train->blocks.end(),
    [&zone](const auto& trainBlockStatus)
    {
      return trainBlockStatus->block->zones->containsObject(zone);
    }) == train->blocks.end())
  {
    auto zoneStatus = zone->getTrainZoneStatus(train);

    zone->trains.removeInternal(zoneStatus);
    train->zones.removeInternal(zoneStatus);

    zoneStatus->destroy();
#ifndef NDEBUG
    std::weak_ptr<TrainZoneStatus> zoneStatusWeak = zoneStatus;
    zoneStatus.reset();
    assert(zoneStatusWeak.expired());
#endif

    return true; // removed
  }
  return false;
}

void TrainTracking::trainEnteredOrLeftZone(Train& train, Zone& zone)
{
  // Re-evaluate zone limitations:
  if(zone.mute)
  {
    train.updateMute();
  }
  if(zone.noSmoke)
  {
    train.updateNoSmoke();
  }
  if(zone.speedLimit.hasLimit())
  {
    train.updateSpeedLimit();
  }
}
