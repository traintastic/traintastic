/**
 * server/src/board/map/connector.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_BOARD_MAP_CONNECTOR_HPP
#define TRAINTASTIC_SERVER_BOARD_MAP_CONNECTOR_HPP

#include <cstdint>
#include <type_traits>
#include <traintastic/board/tilelocation.hpp>
#include "../../enum/tilerotate.hpp"

struct Connector
{
  enum class Direction : uint8_t
  {
    North = 1,
    NorthEast = 2,
    East = 3,
    SouthEast = 4,
    South = 5,
    SouthWest = 6,
    West = 7,
    NorthWest = 8,
  };

  enum class Type : uint8_t
  {
    Rail = 1,
    Road = 2,
  };

  TileLocation location;
  Direction direction;
  Type type;

  Connector(TileLocation location_, Direction direction_, Type type_);
  Connector(TileLocation location_, TileRotate rotate, Type type_);

  bool operator ==(const Connector& other) const
  {
    return
      location == other.location &&
      direction == other.direction &&
      type == other.type;
  }

  Connector opposite() const;
};

constexpr Connector::Direction operator ~(Connector::Direction value)
{
  const auto n = static_cast<std::underlying_type_t<Connector::Direction>>(value);
  return static_cast<Connector::Direction>(n <= 4 ? n + 4 : n - 4);
}

constexpr Connector::Direction rotate90cw(Connector::Direction value)
{
  const auto n = static_cast<std::underlying_type_t<Connector::Direction>>(value);
  return static_cast<Connector::Direction>(n <= 6 ? n + 2 : n - 6);
}

constexpr bool isIntercardinal(Connector::Direction value)
{
  return (static_cast<std::underlying_type_t<Connector::Direction>>(value) & 1) == 0;
}

constexpr Connector::Direction toConnectorDirection(TileRotate value)
{
  const auto r = static_cast<std::underlying_type_t<TileRotate>>(value);
  return static_cast<Connector::Direction>(r < 4 ? r + 5 : r - 3);
}

constexpr TileLocation operator +(TileLocation location, Connector::Direction direction)
{
  switch(direction)
  {
    case Connector::Direction::North:
      return {location.x, static_cast<int16_t>(location.y - 1)};

    case Connector::Direction::NorthEast:
      return {static_cast<int16_t>(location.x + 1), static_cast<int16_t>(location.y - 1)};

    case Connector::Direction::East:
      return {static_cast<int16_t>(location.x + 1), location.y};

    case Connector::Direction::SouthEast:
      return {static_cast<int16_t>(location.x + 1), static_cast<int16_t>(location.y + 1)};

    case Connector::Direction::South:
      return {location.x, static_cast<int16_t>(location.y + 1)};

    case Connector::Direction::SouthWest:
      return {static_cast<int16_t>(location.x - 1), static_cast<int16_t>(location.y + 1)};

    case Connector::Direction::West:
      return {static_cast<int16_t>(location.x - 1), location.y};

    case Connector::Direction::NorthWest:
      return {static_cast<int16_t>(location.x - 1), static_cast<int16_t>(location.y - 1)};
  }
  return location;
}

#endif
