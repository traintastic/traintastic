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

#include "dispatcher.hpp"
#include "../board/tile/rail/blockrailtile.hpp"
#include "../board/pathfinder/trainpathfinder.hpp"
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../log/log.hpp"
#include "../route/trainroute.hpp"
#include "../route/trainrouteentry.hpp"
#include "../train/train.hpp"
#include "../world/world.hpp"

void Dispatcher::trainEmergencyStopChanged(const Train& train)
{
  LOG_DEBUG("Dispatcher::trainEmergencyStopChanged", train.name.value());

  if(train.emergencyStop)
  {
    releaseUnusedPaths(train);
  }
  else
  {
    evaluateTrain(train);
  }
}

void Dispatcher::trainRouteChanged(const Train& train)
{
  LOG_DEBUG("Dispatcher::trainRouteChanged", train.name.value());

  if(!train.route && train.isStopped)
  {
    releaseUnusedPaths(train);
  }
  else
  {
    evaluateTrain(train);
  }
}

void Dispatcher::trainThrottleChanged(const Train& train)
{
  LOG_DEBUG("Dispatcher::trainThrottleChanged", train.name.value());

  evaluateTrain(train);
}

void Dispatcher::trainEnteredBlock(const Train& train, const BlockRailTile& block)
{
  LOG_DEBUG("Dispatcher::trainEnteredBlock", train.name.value(), block.name.value());
  (void)block;

  evaluateTrain(train);
}

void Dispatcher::evaluateTrain(const Train& train)
{
  LOG_DEBUG("Dispatcher::evaluateTrain", train.name.value());

  auto& world = train.world();

  if(train.emergencyStop || !train.route || (train.automatic && !world.automatic))
  {
    return; // don't reserve new paths
  }

  const uint32_t routePosition = train.routePosition.value();
  if(routePosition == train.route->entriesResolved.size() - 1)
  {
    return; // no need to reserve, train is at destination
  }

  const auto& currentEntry = *train.route->entriesResolved[routePosition];
  const auto& nextEntry = *train.route->entriesResolved[routePosition + 1];

  if(auto path = currentEntry.block->getReservedPath(currentEntry.blockTrainDirection() == BlockTrainDirection::TowardsA ? BlockSide::A : BlockSide::B))
  {
    LOG_DEBUG("Dispatcher::evaluateTrain -> path is reserved"); // path reserved
  }
  else if(world.trainPathFinder->reserve(currentEntry.block.value(), currentEntry.blockTrainDirection(), nextEntry.block.value(), nextEntry.blockTrainDirection()))
  {
    LOG_DEBUG("Dispatcher::evaluateTrain -> path reserved successfully");
  }
  else
  {
    LOG_DEBUG("Dispatcher::evaluateTrain -> reserving path failed");
  }
}

void Dispatcher::releaseUnusedPaths(const Train& train)
{
  LOG_DEBUG("Dispatcher::releaseUnusedPaths", train.name.value());

  (void)train;
}

