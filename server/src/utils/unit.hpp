/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
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

#ifndef TRAINTASTIC_SERVER_UTILS_UNIT_HPP
#define TRAINTASTIC_SERVER_UTILS_UNIT_HPP

#include <string_view>

struct Unit
{
  static constexpr std::string_view ampere{"A"};
  static constexpr std::string_view degreeCelcius{"\u00B0C"};
  static constexpr std::string_view milliSeconds{"ms"};
  static constexpr std::string_view percent{"%"};
  static constexpr std::string_view seconds{"s"};
  static constexpr std::string_view volt{"C"};
};

#endif
