/**
 * shared/src/traintastic/enum/lengthunit.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_LENGTHUNIT_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_LENGTHUNIT_HPP

#include <cstdint>
#include "enum.hpp"

enum class LengthUnit
{
  Meter = 0,
  CentiMeter = 1,
  MilliMeter = 2,
  Yard = 3,
  Foot = 4,
  Inch = 5,
};

TRAINTASTIC_ENUM(LengthUnit, "length_unit", 6,
{
  {LengthUnit::Meter, "m"},
  {LengthUnit::CentiMeter, "cm"},
  {LengthUnit::MilliMeter, "mm"},
  {LengthUnit::Yard, "yd"},
  {LengthUnit::Foot, "ft"},
  {LengthUnit::Inch, "in"},
});

#endif
