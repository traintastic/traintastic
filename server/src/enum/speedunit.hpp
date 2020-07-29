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

#ifndef TRAINTASTIC_SERVER_ENUM_SPEEDUNIT_HPP
#define TRAINTASTIC_SERVER_ENUM_SPEEDUNIT_HPP

#include <traintastic/enum/speedunit.hpp>
#include "../core/convertunit.hpp"

template<>
constexpr double convertUnit(double value, SpeedUnit from, SpeedUnit to)
{
  const double ratioKiloMeterPerHour = 3.6;
  const double ratioMilePerHour = 3600 / 1609.344;

  if(from == to)
    return value;

  // convert <from> to m/s:
  switch(from)
  {
    case SpeedUnit::MeterPerSecond:
      break;

    case SpeedUnit::KiloMeterPerHour:
      value /= ratioKiloMeterPerHour;
      break;

    case SpeedUnit::MilePerHour:
      value /= ratioMilePerHour;
      break;
  }

  // convert m/s to <to>:
  switch(to)
  {
    case SpeedUnit::MeterPerSecond:
      break;

    case SpeedUnit::KiloMeterPerHour:
      value *= ratioKiloMeterPerHour;
      break;

    case SpeedUnit::MilePerHour:
      value *= ratioMilePerHour;
      break;
  }

  return value;
}

#endif
