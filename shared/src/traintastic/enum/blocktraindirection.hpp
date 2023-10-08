/**
 * shared/src/traintastic/enum/blocktraindirection.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_BLOCKTRAINDIRECTION_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_BLOCKTRAINDIRECTION_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class BlockTrainDirection : uint8_t
{
  Unknown = 0,
  TowardsA = 1, //!< West for horizontal blocks, South for vertical blocks.
  TowardsB = 2, //!< East for horizontal blocks, North for vertical blocks.
};

TRAINTASTIC_ENUM(BlockTrainDirection, "block_train_direction", 3,
{
  {BlockTrainDirection::Unknown, "unknown"},
  {BlockTrainDirection::TowardsA, "towards_a"},
  {BlockTrainDirection::TowardsB, "towards_b"}
});

constexpr std::array<BlockTrainDirection, 3> blockTrainDirectionValues
{
  BlockTrainDirection::Unknown,
  BlockTrainDirection::TowardsA,
  BlockTrainDirection::TowardsB,
};

constexpr BlockTrainDirection operator !(BlockTrainDirection value)
{
  if(value == BlockTrainDirection::Unknown)
    return BlockTrainDirection::Unknown;

  return (value == BlockTrainDirection::TowardsA) ? BlockTrainDirection::TowardsB : BlockTrainDirection::TowardsA;
}

#endif
