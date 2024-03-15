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
  auto p = std::make_shared<TrainBlockStatus>(id.empty() ? world.getUniqueId(TrainBlockStatus::classId) : std::string{id});
  p->block.setValueInternal(block_.shared_ptr<BlockRailTile>());
  p->train.setValueInternal(train_.shared_ptr<Train>());
  p->direction.setValueInternal(direction_);
  TrainBlockStatus::addToWorld(world, *p);
  return p;
}

std::shared_ptr<TrainBlockStatus> TrainBlockStatus::create(BlockRailTile& block_, std::string identification_, BlockTrainDirection direction_, std::string_view id)
{
  World& world = block_.world();
  auto p = std::make_shared<TrainBlockStatus>(id.empty() ? world.getUniqueId(TrainBlockStatus::classId) : std::string{id});
  p->block.setValueInternal(block_.shared_ptr<BlockRailTile>());
  p->identification.setValueInternal(std::move(identification_));
  p->direction.setValueInternal(direction_);
  TrainBlockStatus::addToWorld(world, *p);
  return p;
}

TrainBlockStatus::TrainBlockStatus(std::string id)
  : StateObject(std::move(id))
  , block{this, "block", nullptr, PropertyFlags::ReadOnly | PropertyFlags::StoreState}
  , train{this, "train", nullptr, PropertyFlags::ReadOnly | PropertyFlags::StoreState}
  , identification{this, "identification", "", PropertyFlags::ReadOnly | PropertyFlags::StoreState}
  , direction{this, "direction", BlockTrainDirection::TowardsA, PropertyFlags::ReadOnly | PropertyFlags::StoreState}
{
  m_interfaceItems.add(block);

  m_interfaceItems.add(train);

  m_interfaceItems.add(identification);

  Attributes::addValues(direction, blockTrainDirectionValues);
  m_interfaceItems.add(direction);
}

void TrainBlockStatus::destroying()
{
  auto self = shared_ptr<TrainBlockStatus>();
  if(block)
    block->trains.removeInternal(self);
  if(train)
    train->blocks.removeInternal(self);
  removeFromWorld(block->world(), *this);
  StateObject::destroying();
}
