/**
 * server/src/board/tile/rail/turnout/turnout3wayrailtile.cpp
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

#include "turnout3wayrailtile.hpp"
#include "../../../../core/objectproperty.tpp"
#include "../../../../core/attributes.hpp"
#include "../../../../hardware/output/outputcontroller.hpp"

static const std::array<TurnoutPosition, 4> positionValues = {TurnoutPosition::Unknown, TurnoutPosition::Straight, TurnoutPosition::Left, TurnoutPosition::Right};

static std::optional<OutputActionValue> getDefaultActionValue(TurnoutPosition turnoutPosition, OutputType outputType, size_t outputIndex)
{
  // FIXME: implement more defaults
  switch(outputType)
  {
    case OutputType::Aspect:
      if(outputIndex == 0)
      {
        // There is no official/defacto standard yet, until there is use values used by YaMoRC YD8116.
        switch(turnoutPosition)
        {
          case TurnoutPosition::Left:
            return static_cast<int16_t>(0);

          case TurnoutPosition::Right:
            return static_cast<int16_t>(1);

          case TurnoutPosition::Straight:
            return static_cast<int16_t>(16);

          default:
            break;
        }
      }
      break;

    default:
      break;
  }
  return {};
}

Turnout3WayRailTile::Turnout3WayRailTile(World& world, std::string_view _id)
  : TurnoutRailTile(world, _id, TileId::RailTurnout3Way, 4)
{
  // Skip Unknown position
  std::span<const TurnoutPosition, 3> setPositionValues = std::span(positionValues).subspan<1>();

  outputMap.setValueInternal(std::make_shared<TurnoutOutputMap>(*this, outputMap.name(), std::initializer_list<TurnoutPosition>{TurnoutPosition::Straight, TurnoutPosition::Left, TurnoutPosition::Right}, getDefaultActionValue));

  Attributes::addValues(position, positionValues);
  m_interfaceItems.add(position);

  Attributes::addValues(setPosition, setPositionValues);
  m_interfaceItems.add(setPosition);

  connectOutputMap();
}

void Turnout3WayRailTile::getConnectors(std::vector<Connector>& connectors) const
{
  connectors.emplace_back(location(), rotate, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg135, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg180, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg225, Connector::Type::Rail);
}
