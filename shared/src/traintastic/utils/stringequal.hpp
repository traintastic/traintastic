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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_UTILS_STRINGEQUAL_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_UTILS_STRINGEQUAL_HPP

#include <string>
#include <string_view>

struct StringEqual
{
  using is_transparent = void;

  bool operator()(std::string_view lhs, std::string_view rhs) const noexcept
  {
    return lhs == rhs;
  }

  bool operator()(const char* lhs, std::string_view rhs) const noexcept
  {
    return std::string_view(lhs) == rhs;
  }

  bool operator()(std::string_view lhs, const char* rhs) const noexcept
  {
    return lhs == std::string_view(rhs);
  }
};

#endif
