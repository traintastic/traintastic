/**
 * server/src/utils/category.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_UTILS_CATEGORY_HPP
#define TRAINTASTIC_SERVER_UTILS_CATEGORY_HPP

#include <string_view>

namespace Category
{
  constexpr std::string_view block = "category:block";
  constexpr std::string_view cargo = "category:cargo";
  constexpr std::string_view colors = "category:colors";
  constexpr std::string_view colorsAndAlignment = "category:colors_and_alignment";
  constexpr std::string_view debug = "category:debug";
  constexpr std::string_view developer = "category:developer";
  constexpr std::string_view general = "category:general";
  constexpr std::string_view info = "category:info";
  constexpr std::string_view input = "category:input";
  constexpr std::string_view log = "category:log";
  constexpr std::string_view network = "category:network";
  constexpr std::string_view options = "category:options";
  constexpr std::string_view trains = "category:trains";
  constexpr std::string_view zones = "category:zones";
}

#endif
