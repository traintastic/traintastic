/**
 * server/src/enum/worldscale.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_ENUM_WORLDSCALE_HPP
#define TRAINTASTIC_SERVER_ENUM_WORLDSCALE_HPP

#include <traintastic/enum/worldscale.hpp>
#include <array>

inline constexpr std::array<WorldScale, 5> WorldScaleValues{{
  WorldScale::H0,
  WorldScale::TT,
  WorldScale::N,
  WorldScale::Z,
  WorldScale::Custom,
}};

constexpr double getScaleRatio(WorldScale value)
{
  switch(value)
  {
    case WorldScale::H0:
      return 87;

    case WorldScale::TT:
      return 120;

    case WorldScale::N:
      return 160;

    case WorldScale::Z:
      return 220;

    case WorldScale::Custom:
      break;
  }
  return 0;
}

#endif
