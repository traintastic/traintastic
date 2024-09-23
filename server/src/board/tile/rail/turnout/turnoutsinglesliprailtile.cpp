/**
 * server/src/board/tile/rail/turnout/turnoutsinglesliprailtile.cpp
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

#include "turnoutsinglesliprailtile.hpp"
#include "../../../../core/attributes.hpp"
#include "../../../../core/objectproperty.tpp"
#include "../../../../hardware/output/outputcontroller.hpp"

namespace PositionValues
{
  static const std::array<TurnoutPosition, 3> singleMotor = {
    TurnoutPosition::Unknown,
    TurnoutPosition::Crossed, TurnoutPosition::Diverged
  };
  static const std::array<TurnoutPosition, 4> dualMotor = {
    TurnoutPosition::Unknown,
    TurnoutPosition::Diverged, TurnoutPosition::DoubleSlipStraightA, TurnoutPosition::DoubleSlipStraightB
  };
}

static constexpr tcb::span<const TurnoutPosition> positionValuesSingleMotor = tcb::make_span(PositionValues::singleMotor);
static constexpr tcb::span<const TurnoutPosition> positionValuesDualMotor = tcb::make_span(PositionValues::dualMotor);
static constexpr tcb::span<const TurnoutPosition> setPositionValuesSingleMotor = tcb::make_span(PositionValues::singleMotor).subspan<1>();
static constexpr tcb::span<const TurnoutPosition> setPositionValuesDualMotor = tcb::make_span(PositionValues::dualMotor).subspan<1>();

TurnoutSingleSlipRailTile::TurnoutSingleSlipRailTile(World& world, std::string_view _id)
  : TurnoutSlipRailTile(world, _id, TileId::RailTurnoutSingleSlip)
{
  outputMap.setValueInternal(std::make_shared<TurnoutOutputMap>(*this, outputMap.name(),
                                                                  std::initializer_list<TurnoutPosition>{
                                                                    TurnoutPosition::Crossed, TurnoutPosition::Diverged,
                                                                    TurnoutPosition::DoubleSlipStraightA, TurnoutPosition::DoubleSlipStraightB},
                                                                  getDefaultActionValue));

  Attributes::addValues(position, positionValuesSingleMotor);
  m_interfaceItems.add(position);

  Attributes::addValues(setPosition, setPositionValuesSingleMotor);
  m_interfaceItems.add(setPosition);

  connectOutputMap();
  dualMotorChanged();
}

void TurnoutSingleSlipRailTile::getConnectors(std::vector<Connector>& connectors) const
{
  connectors.emplace_back(location(), rotate, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg135, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg180, Connector::Type::Rail);
  connectors.emplace_back(location(), rotate + TileRotate::Deg315, Connector::Type::Rail);
}

void TurnoutSingleSlipRailTile::dualMotorChanged()
{
  (*outputMap)[TurnoutPosition::Crossed]->visible.setValueInternal(!dualMotor);
  (*outputMap)[TurnoutPosition::DoubleSlipStraightA]->visible.setValueInternal(dualMotor);
  (*outputMap)[TurnoutPosition::DoubleSlipStraightB]->visible.setValueInternal(dualMotor);

  if(dualMotor)
  {
    Attributes::setValues(position, positionValuesDualMotor);
    Attributes::setValues(setPosition, setPositionValuesDualMotor);
  }
  else
  {
    Attributes::setValues(position, positionValuesSingleMotor);
    Attributes::setValues(setPosition, setPositionValuesSingleMotor);
  }
}
