/**
 * server/src/enum/lengthunit.hpp
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

#ifndef TRAINTASTIC_SERVER_ENUM_LENGTHUNIT_HPP
#define TRAINTASTIC_SERVER_ENUM_LENGTHUNIT_HPP

#include <traintastic/enum/lengthunit.hpp>
#include "../core/convertunit.hpp"

template<>
constexpr double convertUnit(double value, LengthUnit from, LengthUnit to)
{
  constexpr double ratioCentiMeter = 0.01;
  constexpr double ratioMilliMeter = 0.001;
  constexpr double ratioYard = 0.9144;
  constexpr double ratioFoot = 0.3048;
  constexpr double ratioInch = 0.0254;

  if(from == to)
    return value;

  // convert <from> to m:
  switch(from)
  {
    case LengthUnit::Meter:
      break;

    case LengthUnit::CentiMeter:
      value *= ratioCentiMeter;
      break;

    case LengthUnit::MilliMeter:
      value *= ratioMilliMeter;
      break;

    case LengthUnit::Yard:
      value *= ratioYard;
      break;

    case LengthUnit::Foot:
      value *= ratioFoot;
      break;

    case LengthUnit::Inch:
      value *= ratioInch;
      break;
  }

  // convert m to <to>:
  switch(to)
  {
    case LengthUnit::Meter:
      break;

    case LengthUnit::CentiMeter:
      value /= ratioCentiMeter;
      break;

    case LengthUnit::MilliMeter:
      value /= ratioMilliMeter;
      break;

    case LengthUnit::Yard:
      value /= ratioYard;
      break;

    case LengthUnit::Foot:
      value /= ratioFoot;
      break;

    case LengthUnit::Inch:
      value /= ratioInch;
      break;
  }

  return value;
}

#endif
