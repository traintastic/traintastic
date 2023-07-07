/**
 * server/src/train/trainblockstatus.hpp
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

#ifndef TRAINTASTIC_SERVER_TRAIN_TRAINBLOCKSTATUS_HPP
#define TRAINTASTIC_SERVER_TRAIN_TRAINBLOCKSTATUS_HPP

#include "../core/stateobject.hpp"
#include "../core/property.hpp"
#include "../core/objectproperty.hpp"
#include <traintastic/enum/blocktraindirection.hpp>

class BlockRailTile;
class Train;

//! \brief Represents the state of a train within a block
class TrainBlockStatus final : public StateObject
{
  CLASS_ID("train_block_status");

public:
  static std::shared_ptr<TrainBlockStatus> create(BlockRailTile& block_, Train& train_, BlockTrainDirection direction_, std::string_view id = {});

  ObjectProperty<BlockRailTile> block;
  ObjectProperty<Train> train;
  Property<BlockTrainDirection> direction; //!< \brief Train direction from the block perspective

  TrainBlockStatus(BlockRailTile& block_, Train& train_, BlockTrainDirection direction_, std::string id);
};

#endif
