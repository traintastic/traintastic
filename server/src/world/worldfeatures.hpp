/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_WORLD_WORLDFEATURES_HPP
#define TRAINTASTIC_SERVER_WORLD_WORLDFEATURES_HPP

#include <cstdint>
#include <array>

enum class WorldFeature
{
  Scripting = 0,
  Boosters = 1,
  TrackDriverSystem = 2,
};

class WorldFeatures
{
public:
  constexpr WorldFeatures() noexcept
    : m_features{0}
  {
  }

  constexpr WorldFeatures(std::initializer_list<WorldFeature> features) noexcept
    : WorldFeatures()
  {
    for(auto f : features)
    {
      set(f, true);
    }
  }

  constexpr bool operator [](WorldFeature f) const noexcept
  {
    return (m_features[index(f)] & bit(f)) != 0;
  }

  constexpr void set(WorldFeature f, bool enable) noexcept
  {
    if(enable)
    {
      m_features[index(f)] |= bit(f);
    }
    else
    {
      m_features[index(f)] &= ~bit(f);
    }
  }

private:
  using ET = uint8_t;
  using UT = std::underlying_type_t<WorldFeature>;

  std::array<ET, 1> m_features;

  static constexpr std::size_t index(WorldFeature f) noexcept
  {
    return static_cast<UT>(f) / (sizeof(ET) * 8);
  }

  static constexpr ET bit(WorldFeature f) noexcept
  {
    return static_cast<ET>(1) << (static_cast<UT>(f) % (sizeof(ET) * 8));
  }
};

constexpr bool isAutomaticFeature(WorldFeature feature)
{
  switch(feature)
  {
    // features controlled by world properties:
    case WorldFeature::Scripting:
      return false;

    // features controlled by other objects:
    case WorldFeature::Boosters:
    case WorldFeature::TrackDriverSystem:
      return true;
  }
  return false;
}

#endif
