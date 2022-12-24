/**
 * server/src/utils/rtrim.hpp
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

#ifndef TRAINTASTIC_SERVER_UTILS_RTRIM_HPP
#define TRAINTASTIC_SERVER_UTILS_RTRIM_HPP

#include <string>
#include <string_view>
#include <algorithm>

constexpr std::string_view rtrim(std::string_view s, char c)
{
  if(s.empty())
    return {};

  size_t size = s.size() - 1;
  while(s.data()[size] == c)
  {
    if(size == 0)
      return {};
    size--;
  }
  return {s.data(), size + 1};
}

inline std::string& rtrim(std::string& s, char c)
{
  const auto sz = s.size();
  size_t n = 0;
  while(n != sz && s[sz - n - 1] == c)
    n++;
  s.resize(sz - n);
  return s;
}

#if __cplusplus >= 202002L
constexpr
#else
inline
#endif
std::string_view rtrim(std::string_view s, std::initializer_list<char> c)
{
  if(s.empty())
    return {};

  size_t size = s.size() - 1;
  while(std::any_of(c.begin(), c.end(), [c1=s.data()[size]](char c2) { return c1 == c2; }))
  {
    if(size == 0)
      return {};
    size--;
  }
  return {s.data(), size + 1};
}

#endif
