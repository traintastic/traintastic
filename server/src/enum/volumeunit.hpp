/**
 * server/src/enum/volumeunit.hpp
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

#ifndef TRAINTASTIC_SERVER_ENUM_VOLUMEUNIT_HPP
#define TRAINTASTIC_SERVER_ENUM_VOLUMEUNIT_HPP

#include <traintastic/enum/volumeunit.hpp>
#include "../core/convertunit.hpp"

template<>
constexpr double convertUnit(double value, VolumeUnit from, VolumeUnit to)
{
  const double ratioCubicMeter = 1000;
  const double ratioGallonUS = 3.785411784;
  const double ratioGallonUK = 4.54609;

  if(from == to)
    return value;

  // convert <from> to l:
  switch(from)
  {
    case VolumeUnit::Liter:
      break;

    case VolumeUnit::CubicMeter:
      value *= ratioCubicMeter;
      break;

    case VolumeUnit::GallonUS:
      value *= ratioGallonUS;
      break;

    case VolumeUnit::GallonUK:
      value *= ratioGallonUK;
      break;
  }

  // convert l to <to>:
  switch(to)
  {
    case VolumeUnit::Liter:
      break;

    case VolumeUnit::CubicMeter:
      value /= ratioCubicMeter;
      break;

    case VolumeUnit::GallonUS:
      value /= ratioGallonUS;
      break;

    case VolumeUnit::GallonUK:
      value /= ratioGallonUK;
      break;
  }

  return value;
}

#endif
