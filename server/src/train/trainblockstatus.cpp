/**
 * server/src/train/trainblockstatus.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "trainblockstatus.hpp"
#include "train.hpp"
#include "../core/objectproperty.tpp"
#include "../core/attributes.hpp"
#include "../board/tile/rail/blockrailtile.hpp"
#include "../world/world.hpp"

std::shared_ptr<TrainBlockStatus> TrainBlockStatus::create(BlockRailTile& block_, Train& train_, BlockTrainDirection direction_, std::string_view id)
{
  World& world = block_.world();
  auto p = std::make_shared<TrainBlockStatus>(block_, train_, direction_, id.empty() ? world.getUniqueId(TrainBlockStatus::classId) : std::string{id});
  TrainBlockStatus::addToWorld(world, *p);
  return p;
}

TrainBlockStatus::TrainBlockStatus(BlockRailTile& block_, Train& train_, BlockTrainDirection direction_, std::string id)
  : StateObject(std::move(id))
  , block{this, "block", block_.shared_ptr<BlockRailTile>(), PropertyFlags::ReadOnly | PropertyFlags::StoreState}
  , train{this, "train", train_.shared_ptr<Train>(), PropertyFlags::ReadOnly | PropertyFlags::StoreState}
  , direction{this, "direction", direction_, PropertyFlags::ReadOnly | PropertyFlags::StoreState}
{
  m_interfaceItems.add(block);

  m_interfaceItems.add(train);

  Attributes::addValues(direction, blockTrainDirectionValues);
  m_interfaceItems.add(direction);
}
