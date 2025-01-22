/**
 * server/src/board/tile/rail/turnout/turnoutsliprailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "turnoutsliprailtile.hpp"
#include "../../../../core/attributes.hpp"

std::optional<OutputActionValue> TurnoutSlipRailTile::getDefaultActionValue(TurnoutPosition turnoutPosition, OutputType outputType, size_t outputIndex)
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

          case TurnoutPosition::DoubleSlipStraightA:
            return static_cast<int16_t>(17);

          case TurnoutPosition::DoubleSlipStraightB:
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

TurnoutSlipRailTile::TurnoutSlipRailTile(World& world, std::string_view _id, TileId tileId_)
  : TurnoutRailTile(world, _id, tileId_, 4)
  , dualMotor{this, "dual_motor", false, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly,
      [this](bool /*value*/)
      {
        dualMotorChanged();
      }}
{
  Attributes::addDisplayName(dualMotor, "board_tile.rail.turnout_slip:dual_motor");
  m_interfaceItems.add(dualMotor);
}

void TurnoutSlipRailTile::loaded()
{
  TurnoutRailTile::loaded();
  dualMotorChanged();
}
