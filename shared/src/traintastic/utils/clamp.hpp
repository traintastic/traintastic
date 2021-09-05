/**
 * shared/src/traintastic/utils/clamp.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_UTILS_CLAMP_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_UTILS_CLAMP_HPP

#include <algorithm>
#include <type_traits>

template<class Tout, class Tin>
constexpr Tout clamp(Tin value)
{
  static_assert(std::is_integral_v<Tin>);
  static_assert(std::is_integral_v<Tout>);
  return static_cast<Tout>(std::clamp<Tin>(value, std::numeric_limits<Tout>::min(), std::numeric_limits<Tout>::max()));
}

#endif
