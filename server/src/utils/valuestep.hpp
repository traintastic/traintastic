/**
 * server/src/utils/valuestep.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_UTILS_VALUESTEP_HPP
#define TRAINTASTIC_SERVER_UTILS_VALUESTEP_HPP

#include <cassert>
#include <cmath>

constexpr double valueStepUp(double value, double step)
{
  assert(step > 0);
  const double margin = step / 1e6;
  return (std::floor((value + margin) / step) + 1) * step;
}

constexpr double valueStepDown(double value, double step)
{
  assert(step > 0);
  const double margin = step / 1e6;
  return (std::ceil((value - margin) / step) - 1) * step;
}

constexpr double valueStep(double value, double step, bool up)
{
  return up ? valueStepUp(value, step) : valueStepDown(value, step);
}

template<typename T>
requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
constexpr T valueStepRound(T value, T step)
{
  if constexpr(std::is_integral_v<T>)
  {
    return ((value / step) + (((value % step) >= step / 2) ? 1 : 0)) * step;
  }
  else // float
  {
    return std::round(value / step) * step;
  }
}

#endif
