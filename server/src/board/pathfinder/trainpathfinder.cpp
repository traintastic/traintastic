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

#include "trainpathfinder.hpp"
#include <queue>
#include <traintastic/enum/blocktraindirection.hpp>
#include <traintastic/enum/turnoutposition.hpp>
#include "../map/blockpath.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../board/tile/rail/blockrailtile.hpp"
#include "../../train/trainblockstatus.hpp"

TrainPathFinder::TrainPathFinder(Object& parent_, std::string_view parentPropertyName)
  : PathFinder(parent_, parentPropertyName)
  , reserve{*this, "reserve", MethodFlags::ScriptCallable,
      [](const std::shared_ptr<BlockRailTile>& fromBlock, BlockTrainDirection fromDirection, const std::shared_ptr<BlockRailTile>& toBlock, BlockTrainDirection toDirection)
      {
        if(!fromBlock || fromBlock->trains.empty() || !isKnown(fromDirection) || !toBlock || !isKnown(toDirection)) [[unlikely]]
        {
          return false;
        }

        const auto fromSide = (fromDirection == BlockTrainDirection::TowardsA) ? BlockSide::A : BlockSide::B;
        const auto toSide = (toDirection == BlockTrainDirection::TowardsB) ? BlockSide::A : BlockSide::B;

        // FIXME: for now only support block to block (direct path only)

        for(const auto& path : fromBlock->paths())
        {
          if(path->fromSide() == fromSide && path->toBlock() == toBlock && path->toSide() == toSide)
          {
            const auto train =
              (fromDirection == BlockTrainDirection::TowardsB)
                ? fromBlock->trains.back()->train.value()
                : fromBlock->trains.front()->train.value();

            return path->reserve(train);
          }
        }

        return false;
      }}
{
  m_interfaceItems.add(reserve);
}

std::vector<BlockRailTile*> TrainPathFinder::find(BlockRailTile& from, std::optional<BlockSide> fromSide, BlockRailTile& to, std::optional<BlockSide>& toSide)
{
  return dijkstra(from, fromSide, to, toSide,
    [](const BlockPath& path, BlockSide enterSide) -> uint64_t
    {
      if(path.fromSide() == enterSide) // Same side -> train reverse
      {
        return 1000;
      }

      uint64_t cost = 1;

      for(const auto& turnout : path.m_turnouts)
      {
        switch(std::get<1>(turnout))
        {
          case TurnoutPosition::Straight:
            cost += 1;
            break;

          case TurnoutPosition::Crossed:
          case TurnoutPosition::DoubleSlipStraightA:
          case TurnoutPosition::DoubleSlipStraightB:
            cost += 2;
            break;

          case TurnoutPosition::Left:
          case TurnoutPosition::Right:
          case TurnoutPosition::Diverged:
            cost += 5;
            break;

          case TurnoutPosition::Unknown: [[unlikely]]
            assert(false);
            break;
        }
      }

      cost += 2 * (path.m_crossings.size() + path.m_crossOvers.size());

      return cost;
    });
}

std::vector<BlockRailTile*> TrainPathFinder::dijkstra(BlockRailTile& from, std::optional<BlockSide> fromSide, BlockRailTile& to, std::optional<BlockSide>& toSide, std::function<uint64_t(const BlockPath&, BlockSide)> getCost)
{
  (void)toSide;

  struct State
  {
    BlockRailTile* block;
    BlockSide side;

    bool operator==(const State& other) const
    {
      return block == other.block && side == other.side;
    }
  };

  struct StateHash
  {
    size_t operator()(const State& state) const
    {
      size_t hash = std::hash<BlockRailTile*>{}(state.block);
      hash ^= std::hash<std::underlying_type_t<BlockSide>>{}(static_cast<std::underlying_type_t<BlockSide>>(state.side));
      return hash;
    }
  };

  struct QueueEntry
  {
    State state;
    uint64_t cost;

    bool operator>(const QueueEntry& other) const
    {
      return cost > other.cost;
    }
  };

  static constexpr auto infinity = std::numeric_limits<uint64_t>::max();

  std::unordered_map<State, uint64_t, StateHash> distance; // cost is used as distance
  std::unordered_map<State, State, StateHash> previous;
  std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<>> queue;

  // start state, search can be in one or both directions:
  for(const BlockSide side : {BlockSide::A, BlockSide::B})
  {
    if(fromSide && *fromSide != side)
    {
      continue;
    }

    State state{&from, side};
    const uint64_t cost = 0;
    distance[state] = cost;
    queue.push({state, cost});
  }

  State destination{nullptr, BlockSide::A};

  while(!queue.empty())
  {
    const QueueEntry current = queue.top();
    queue.pop();

    const auto distanceIt = distance.find(current.state);
    if(distanceIt == distance.end() || current.cost != distanceIt->second)
    {
      continue;
    }

    if(current.state.block == &to && (!toSide || current.state.side == *toSide))
    {
      destination = current.state;
      break;
    }

    for(const auto& path : current.state.block->paths())
    {
      assert(path);
      assert(path->toBlock());

      const State next{path->toBlock().get(), path->toSide()};
      const auto cost = getCost(*path, current.state.side);

      if(current.cost > infinity - cost)
      {
        continue; // maximum reached
      }

      const auto nextCost = current.cost + cost;
      const auto nextDistanceIt = distance.find(next);

      if(nextDistanceIt != distance.end() && nextCost >= nextDistanceIt->second)
      {
        continue; // drop it, higher cost
      }

      distance[next] = nextCost;
      previous[next] = current.state;

      queue.push({next, nextCost});
    }
  }

  if(!destination.block)
  {
    return {}; // no path found
  }

  // reconstruct path backwards:
  std::vector<BlockRailTile*> blocks;

  for(State state = destination;;)
  {
    blocks.emplace_back(state.block);

    if(state.block == &from && distance[state] == 0)
    {
      break;
    }

    const auto previousIt = previous.find(state);

    if(previousIt == previous.end())
    {
      assert(false); // should not happen
      return {};
    }

    state = previousIt->second;
  }

  std::reverse(blocks.begin(), blocks.end());

  toSide = destination.side;

  return blocks;
}
