/**
 * server/src/utils/random.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023,2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_UTILS_RANDOM_HPP
#define TRAINTASTIC_SERVER_UTILS_RANDOM_HPP

#include <ctime>
#include <random>

class Random
{
  private:
    inline static std::mt19937 s_gen{static_cast<unsigned int>(time(nullptr))};

    Random() = default;

  public:
    template<typename T>
    static T value(T min, T max)
    {
      std::uniform_int_distribution<T> dist(min, max);
      return dist(s_gen);
    }

    template<typename T>
    static T value(std::pair<T, T> range)
    {
      return value(range.first, range.second);
    }
};

#endif
