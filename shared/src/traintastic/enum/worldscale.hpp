/**
 * shared/src/enum/worldscale.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_WORLDSCALE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_WORLDSCALE_HPP

#include <cstdint>
#include "enum.hpp"

enum class WorldScale : uint8_t
{
  H0 = 0, //!< 1:87
  N = 1,  //!< 1:160
  TT = 2, //!< 1:120
  Z = 3,  //!< 1:220
  Custom = 255
};

template<>
struct EnumName<WorldScale>
{
  static constexpr char const* value = "world_scale";
};

#endif
