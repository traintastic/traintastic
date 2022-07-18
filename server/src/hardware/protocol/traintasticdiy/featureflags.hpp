/**
 * server/src/hardware/protocol/traintasticdiy/featureflags.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICDIY_FEATUREFLAGS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICDIY_FEATUREFLAGS_HPP

#include <cstdint>

namespace TraintasticDIY {

enum class FeatureFlags1 : uint8_t
{
  None = 0x00,
  Input = 0x01,
  Output = 0x02,
  Throttle = 0x04,
};

enum class FeatureFlags2 : uint8_t
{
  None = 0x00,
};

enum class FeatureFlags3 : uint8_t
{
  None = 0x00,
};

enum class FeatureFlags4 : uint8_t
{
  None = 0x00,
};

template<class T>
constexpr bool isFeatureFlagType()
{
  return
    std::is_same_v<T, FeatureFlags1> ||
    std::is_same_v<T, FeatureFlags2> ||
    std::is_same_v<T, FeatureFlags3> ||
    std::is_same_v<T, FeatureFlags4>;
}

}

template<class T, std::enable_if_t<TraintasticDIY::isFeatureFlagType<T>()>* = nullptr>
constexpr T operator&(const T& lhs, const T& rhs)
{
  using UT = std::underlying_type_t<T>;
  return static_cast<T>(static_cast<UT>(lhs) & static_cast<UT>(rhs));
}

template<class T, std::enable_if_t<TraintasticDIY::isFeatureFlagType<T>()>* = nullptr>
constexpr T operator|(const T& lhs, const T& rhs)
{
  using UT = std::underlying_type_t<T>;
  return static_cast<T>(static_cast<UT>(lhs) | static_cast<UT>(rhs));
}

template<class T, std::enable_if_t<TraintasticDIY::isFeatureFlagType<T>()>* = nullptr>
constexpr bool contains(T set, T value)
{
  return (set & value) == value;
}

#endif
