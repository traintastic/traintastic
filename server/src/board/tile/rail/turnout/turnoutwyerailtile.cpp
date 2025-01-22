/**
 * server/src/board/tile/rail/turnout/turnoutwyerailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2022,2024 Reinder Feenstra
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

#include "turnoutwyerailtile.hpp"
#include "../../../../core/attributes.hpp"
#include "../../../../core/objectproperty.tpp"
#include "../../../../hardware/output/outputcontroller.hpp"

static const std::array<TurnoutPosition, 3> positionValues = {TurnoutPosition::Unknown, TurnoutPosition::Left, TurnoutPosition::Right};

static std::optional<OutputActionValue> getDefaultActionValue(TurnoutPosition turnoutPosition, OutputType outputType, size_t outputIndex)
{
  // FIXME: implement defaults
  (void)turnoutPosition;
  (void)outputType;
  (void)outputIndex;
  return {};
}

TurnoutWyeRailTile::TurnoutWyeRailTile(World& world, std::string_view _id)
  : TurnoutRailTile(world, _id, TileId::RailTurnoutWye, 3)
{
  // Skip Unknown position
  tcb::span<const TurnoutPosition, 2> setPositionValues = tcb::make_span(positionValues).subspan<1>();

  outputMap.setValueInternal(std::make_shared<TurnoutOutputMap>(*this, outputMap.name(), std::initializer_list<TurnoutPosition>{TurnoutPosition::Left, TurnoutPosition::Right}, getDefaultActionValue));

  Attributes::addValues(position, positionValues);
  m_interfaceItems.add(position);

  Attributes::addValues(setPosition, setPositionValues);
  m_interfaceItems.add(setPosition);

  connectOutputMap();
}

void TurnoutWyeRailTile::getConnectors(std::vector<Connector>& connectors) const
{
  connectors.emplace_back(location(), rotate, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg135, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg225, Connector::Type::Rail);
}
