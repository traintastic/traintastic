/**
 * server/src/board/map/path.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "path.hpp"
#include <cassert>
#include <traintastic/enum/turnoutposition.hpp>
#include "node.hpp"
#include "link.hpp"
#include "../tile/rail/turnout/turnoutrailtile.hpp"

const std::shared_ptr<Link>& Path::otherLink(const Node& node, const Link& link)
{
  static const std::shared_ptr<Link> noLink{};
  const auto& links = node.links();
  if(links.size() == 2)
    return links[(links[0].get() == &link) ? 1 : 0];
  assert(false);
  return noLink;
}

tcb::span<const Path::TurnoutPositionLink> Path::getTurnoutLinks(TurnoutRailTile& turnout, const Link& link)
{
  static constexpr std::array<TurnoutPositionLink, 1> straight0{{{TurnoutPosition::Straight, 0}}};
  static constexpr std::array<TurnoutPositionLink, 1> left0{{{TurnoutPosition::Left, 0}}};
  static constexpr std::array<TurnoutPositionLink, 1> right0{{{TurnoutPosition::Right, 0}}};

  const auto& node = turnout.node()->get();

  switch(turnout.tileId())
  {
    case TileId::RailTurnoutLeft45:
    case TileId::RailTurnoutLeft90:
    case TileId::RailTurnoutLeftCurved:
      //  1  2
      //   \ |
      //    \|
      //     0
      if(node.getLink(0).get() == &link)
      {
        static constexpr std::array<TurnoutPositionLink, 2> links{{{TurnoutPosition::Left, 1}, {TurnoutPosition::Straight, 2}}};
        return links;
      }
      else if(node.getLink(1).get() == &link)
      {
        return left0;
      }
      else if(node.getLink(2).get() == &link)
      {
        return straight0;
      }
      break;

    case TileId::RailTurnoutRight45:
    case TileId::RailTurnoutRight90:
    case TileId::RailTurnoutRightCurved:
      //  1  2
      //  | /
      //  |/
      //  0
      if(node.getLink(0).get() == &link)
      {
        static constexpr std::array<TurnoutPositionLink, 2> links{{{TurnoutPosition::Straight, 1}, {TurnoutPosition::Right, 2}}};
        return links;
      }
      else if(node.getLink(1).get() == &link)
      {
        return straight0;
      }
      else if(node.getLink(2).get() == &link)
      {
        return right0;
      }
      break;

    case TileId::RailTurnoutWye:
      //  1     2
      //   \   /
      //    \ /
      //     0
      if(node.getLink(0).get() == &link)
      {
        static constexpr std::array<TurnoutPositionLink, 2> links{{{TurnoutPosition::Left, 1}, {TurnoutPosition::Right, 2}}};
        return links;
      }
      else if(node.getLink(1).get() == &link)
      {
        return left0;
      }
      else if(node.getLink(2).get() == &link)
      {
        return right0;
      }
      break;

    case TileId::RailTurnout3Way:
      //  1  2  3
      //   \ | /
      //    \|/
      //     0
      if(node.getLink(0).get() == &link)
      {
        static constexpr std::array<TurnoutPositionLink, 3> links{{{TurnoutPosition::Left, 1}, {TurnoutPosition::Straight, 2}, {TurnoutPosition::Right, 3}}};
        return links;
      }
      else if(node.getLink(1).get() == &link)
      {
        return left0;
      }
      else if(node.getLink(2).get() == &link)
      {
        return straight0;
      }
      else if(node.getLink(3).get() == &link)
      {
        return right0;
      }
      break;

    case TileId::RailTurnoutDoubleSlip:
    case TileId::RailTurnoutSingleSlip:
      //  Double:      Single:
      //      2            2
      //      |\           |
      //  1 --+-- 3    1 --+-- 3
      //     \|           \|
      //      0            0
      if(node.getLink(0).get() == &link)
      {
        static constexpr std::array<TurnoutPositionLink, 2> links{{{TurnoutPosition::DoubleSlipStraightA, 2}, {TurnoutPosition::Left, 1}}};
        return links;
      }
      else if(node.getLink(1).get() == &link)
      {
        static constexpr std::array<TurnoutPositionLink, 2> links{{{TurnoutPosition::DoubleSlipStraightA, 3}, {TurnoutPosition::Left, 0}}};
        return links;
      }
      else if(node.getLink(2).get() == &link)
      {
        if(turnout.tileId() == TileId::RailTurnoutSingleSlip)
        {
          static constexpr std::array<TurnoutPositionLink, 1> linksSingle{{{TurnoutPosition::DoubleSlipStraightB, 0}}};
          return linksSingle;
        }
        static constexpr std::array<TurnoutPositionLink, 2> linksDouble{{{TurnoutPosition::DoubleSlipStraightB, 0}, {TurnoutPosition::Right, 3}}};
        return linksDouble;
      }
      else if(node.getLink(3).get() == &link)
      {
        if(turnout.tileId() == TileId::RailTurnoutSingleSlip)
        {
          static constexpr std::array<TurnoutPositionLink, 1> linksSingle{{{TurnoutPosition::DoubleSlipStraightB, 1}}};
          return linksSingle;
        }
        static constexpr std::array<TurnoutPositionLink, 2> linksDouble{{{TurnoutPosition::DoubleSlipStraightB, 1}, {TurnoutPosition::Right, 2}}};
        return linksDouble;
      }
      break;

    default:
      break;
  }

  assert(false);
  return {};
}
