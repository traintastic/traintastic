/**
 * shared/src/traintastic/enum/weightunit.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_WEIGHTUNIT_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_WEIGHTUNIT_HPP

#include <cstdint>
#include "enum.hpp"

enum class WeightUnit
{
  KiloGram = 0,
  Ton = 1,
  ShortTons = 2,
  LongTons = 3,
};

TRAINTASTIC_ENUM(WeightUnit, "weight_unit", 4,
{
  {WeightUnit::KiloGram, "kg"},
  {WeightUnit::Ton, "ton"},
  {WeightUnit::ShortTons, "short_tons"},
  {WeightUnit::LongTons, "long_tons"},
});

#endif
