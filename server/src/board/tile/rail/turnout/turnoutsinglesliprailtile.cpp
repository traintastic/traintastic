/**
 * server/src/board/tile/rail/turnout/turnoutsinglesliprailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2022 Reinder Feenstra
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

#include "turnoutsinglesliprailtile.hpp"
#include "../../../../core/attributes.hpp"
#include "../../../../core/objectproperty.tpp"

static const std::array<TurnoutPosition, 5> positionValues = {TurnoutPosition::Crossed, TurnoutPosition::Diverged,
                                                              TurnoutPosition::DoubleSlipStraightA, TurnoutPosition::DoubleSlipStraightB,
                                                              TurnoutPosition::Unknown};
static const std::array<TurnoutPosition, 4> setPositionValues = {TurnoutPosition::Crossed, TurnoutPosition::Diverged,
                                                                 TurnoutPosition::DoubleSlipStraightA, TurnoutPosition::DoubleSlipStraightB};

TurnoutSingleSlipRailTile::TurnoutSingleSlipRailTile(World& world, std::string_view _id)
  : TurnoutRailTile(world, _id, TileId::RailTurnoutSingleSlip, 4)
{
  outputMap.setValueInternal(std::make_shared<TurnoutOutputMap>(*this, outputMap.name(),
                                                                  std::initializer_list<TurnoutPosition>{
                                                                    TurnoutPosition::Crossed, TurnoutPosition::Diverged,
                                                                    TurnoutPosition::DoubleSlipStraightA, TurnoutPosition::DoubleSlipStraightB}));

  Attributes::addValues(position, positionValues);
  m_interfaceItems.add(position);

  Attributes::addValues(setPosition, setPositionValues);
  m_interfaceItems.add(setPosition);
}

void TurnoutSingleSlipRailTile::getConnectors(std::vector<Connector>& connectors) const
{
  connectors.emplace_back(location(), rotate, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg135, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg180, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg315, Connector::Type::Rail);
}
