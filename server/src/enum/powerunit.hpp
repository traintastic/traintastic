/**
 * server/src/enum/powerunit.hpp
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

#ifndef TRAINTASTIC_SERVER_ENUM_POWERUNIT_HPP
#define TRAINTASTIC_SERVER_ENUM_POWERUNIT_HPP

#include <traintastic/enum/powerunit.hpp>
#include "../core/convertunit.hpp"

template<>
constexpr double convertUnit(double value, PowerUnit from, PowerUnit to)
{
  const double ratioKiloWatt = 1'000;
  const double ratioMegaWatt = 1'000'000;
  const double ratioHorsePower = 745.7;

  if(from == to)
    return value;

  // convert <from> to W:
  switch(from)
  {
    case PowerUnit::Watt:
      break;

    case PowerUnit::KiloWatt:
      value *= ratioKiloWatt;
      break;

    case PowerUnit::MegaWatt:
      value *= ratioMegaWatt;
      break;

    case PowerUnit::HorsePower:
      value *= ratioHorsePower;
      break;
  }

  // convert W to <to>:
  switch(to)
  {
    case PowerUnit::Watt:
      break;

    case PowerUnit::KiloWatt:
      value /= ratioKiloWatt;
      break;

    case PowerUnit::MegaWatt:
      value /= ratioMegaWatt;
      break;

    case PowerUnit::HorsePower:
      value /= ratioHorsePower;
      break;
  }

  return value;
}

#endif
