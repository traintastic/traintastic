/**
 * server/src/board/tile/rail/turnout/turnoutrightrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2022,2024-2025 Reinder Feenstra
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

#include "turnoutrightrailtile.hpp"
#include "../../../../core/attributes.hpp"
#include "../../../../core/objectproperty.tpp"
#include "../../../../hardware/output/outputcontroller.hpp"

static const std::array<TurnoutPosition, 3> positionValues = {TurnoutPosition::Unknown, TurnoutPosition::Straight, TurnoutPosition::Right};

static std::optional<OutputActionValue> getDefaultActionValue(TurnoutPosition turnoutPosition, OutputType outputType, size_t outputIndex)
{
  if(outputIndex == 0)
  {
    switch(outputType)
    {
      case OutputType::Pair:
        if(turnoutPosition == TurnoutPosition::Straight)
        {
          return PairOutputAction::Second;
        }
        else if(turnoutPosition == TurnoutPosition::Right)
        {
          return PairOutputAction::First;
        }
        break;

      case OutputType::Aspect:
        // YaMoRC YD8116 defaults aspects:
        if(turnoutPosition == TurnoutPosition::Straight)
        {
          return static_cast<int16_t>(0);
        }
        else if(turnoutPosition == TurnoutPosition::Right)
        {
          return static_cast<int16_t>(16);
        }
        break;

      default:
        break;
    }
  }
  return {};
}

TurnoutRightRailTile::TurnoutRightRailTile(World& world, std::string_view _id, TileId tileId_)
  : TurnoutRailTile(world, _id, tileId_, 3)
{
  // Skip Unknown position
  std::span<const TurnoutPosition, 2> setPositionValues = std::span(positionValues).subspan<1>();

  outputMap.setValueInternal(std::make_shared<TurnoutOutputMap>(*this, outputMap.name(), std::initializer_list<TurnoutPosition>{TurnoutPosition::Straight, TurnoutPosition::Right}, getDefaultActionValue));

  Attributes::addValues(position, positionValues);
  m_interfaceItems.add(position);

  Attributes::addValues(setPosition, setPositionValues);
  m_interfaceItems.add(setPosition);

  connectOutputMap();
}
