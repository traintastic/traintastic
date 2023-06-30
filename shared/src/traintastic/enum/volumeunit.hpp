/**
 * shared/src/traintastic/enum/volumeunit.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_VOLUMEUNIT_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_VOLUMEUNIT_HPP

#include <cstdint>
#include "enum.hpp"

enum class VolumeUnit
{
  Liter = 0,
  CubicMeter = 1,
  GallonUS = 2,
  GallonUK = 3,
};

TRAINTASTIC_ENUM(VolumeUnit, "volume_unit", 4,
{
  {VolumeUnit::Liter, "l"},
  {VolumeUnit::CubicMeter, "m3"},
  {VolumeUnit::GallonUS, "gallon_us"},
  {VolumeUnit::GallonUK, "gallon_uk"},
});

#endif
