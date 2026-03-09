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
#include <traintastic/enum/blocktraindirection.hpp>
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


