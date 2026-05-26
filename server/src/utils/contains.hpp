/**
 * server/src/utils/contains.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2024-2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_UTILS_CONTAINS_HPP
#define TRAINTASTIC_SERVER_UTILS_CONTAINS_HPP

#include <array>
#include <vector>
#include <span>

template<class T, std::size_t N>
inline bool contains(const std::array<T, N>& array, T value)
{
  return std::find(array.begin(), array.end(), value) != array.end();
}

template<class T>
inline bool contains(const std::vector<T>& vector, T value)
{
  return std::find(vector.begin(), vector.end(), value) != vector.end();
}

template<class T>
inline bool contains(std::span<const T> span, T value)
{
  return std::find(span.begin(), span.end(), value) != span.end();
}

#endif
