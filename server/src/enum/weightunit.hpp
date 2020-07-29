/**
 * server/src/enum/weightunit.hpp
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

#ifndef TRAINTASTIC_SERVER_ENUM_WEIGHTUNIT_HPP
#define TRAINTASTIC_SERVER_ENUM_WEIGHTUNIT_HPP

#include <traintastic/enum/weightunit.hpp>
#include "../core/convertunit.hpp"

template<>
constexpr double convertUnit(double value, WeightUnit from, WeightUnit to)
{
  const double ratioTon = 1000;
  const double ratioShortTons = 907.18474;
  const double ratioLongTons = 1016.04691;

  if(from == to)
    return value;

  // convert <from> to kg:
  switch(from)
  {
    case WeightUnit::KiloGram:
      break;

    case WeightUnit::Ton:
      value *= ratioTon;
      break;

    case WeightUnit::ShortTons:
      value *= ratioShortTons;
      break;

    case WeightUnit::LongTons:
      value *= ratioLongTons;
      break;
  }

  // convert kg to <to>:
  switch(to)
  {
    case WeightUnit::KiloGram:
      break;

    case WeightUnit::Ton:
      value /= ratioTon;
      break;

    case WeightUnit::ShortTons:
      value /= ratioShortTons;
      break;

    case WeightUnit::LongTons:
      value /= ratioLongTons;
      break;
  }

  return value;
}

#endif
