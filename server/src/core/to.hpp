/**
 * server/src/core/to.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#ifndef SERVER_CORE_TO_HPP
#define SERVER_CORE_TO_HPP

#include <type_traits>
#include <limits>
#include <stdexcept>
#include <cmath>

class conversion_error : public std::runtime_error
{
  public:
    conversion_error() :
      std::runtime_error("conversion error")
    {
    }
};

class out_of_range_error : public std::runtime_error
{
  public:
    out_of_range_error() :
      std::runtime_error("out of range error")
    {
    }
};

template<typename To, typename From>
To to(const From& value)
{
  if constexpr(std::is_same<To, From>::value)
    return value;
  else if constexpr(!std::is_same<To, bool>::value && std::is_integral<To>::value && !std::is_same<From, bool>::value && std::is_integral<From>::value)
  {
    if constexpr(std::numeric_limits<To>::min() <= std::numeric_limits<From>::min() && std::numeric_limits<To>::max() >= std::numeric_limits<From>::max())
      return value;
    else if(value >= std::numeric_limits<To>::min() && value <= std::numeric_limits<To>::max())
      return value;
    else
      throw out_of_range_error();
  }
  else if constexpr(std::is_floating_point<To>::value && (std::is_integral<From>::value || std::is_floating_point<From>::value))
    return value;
  else if constexpr(std::is_integral<To>::value && std::is_floating_point<From>::value)
  {
    if(value >= std::numeric_limits<To>::min() && value <= std::numeric_limits<To>::max())
      return static_cast<To>(std::round(value));
    else
      throw out_of_range_error();
  }

  throw conversion_error();
}

#endif
