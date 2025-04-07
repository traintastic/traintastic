/**
 * server/src/board/map/path.hpp
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

#ifndef TRAINTASTIC_SERVER_BOARD_MAP_PATH_HPP
#define TRAINTASTIC_SERVER_BOARD_MAP_PATH_HPP

#include <memory>
#include <span>

class TurnoutRailTile;
enum class TurnoutPosition : uint8_t;
class Node;
class Link;

class Path
{
protected:
  struct TurnoutPositionLink
  {
    TurnoutPosition turnoutPosition;
    uint8_t linkIndex;
  };

  static const std::shared_ptr<Link>& otherLink(const Node& node, const Link& link);
  static std::span<const TurnoutPositionLink> getTurnoutLinks(TurnoutRailTile& turnout, const Link& link);
};

#endif
