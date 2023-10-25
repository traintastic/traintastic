/**
 * server/src/board/map/blockpath.hpp
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

#ifndef TRAINTASTIC_SERVER_BOARD_MAP_BLOCKPATH_HPP
#define TRAINTASTIC_SERVER_BOARD_MAP_BLOCKPATH_HPP

#include "path.hpp"

#include <memory>
#include <array>
#include <vector>
#include <utility>

class BlockRailTile;
class CrossRailTile;
enum class CrossState : uint8_t;
class DirectionControlRailTile;
enum class DirectionControlState : uint8_t;
class SignalRailTile;
class NXButtonRailTile;
class Node;
class Link;

/**
 * \brief A path between two blocks
 */
class BlockPath : public Path
{
  public:
    enum class Side
    {
      A = 0,
      B = 1,
    };

  private:
    BlockRailTile const& m_fromBlock;
    const Side m_fromSide;
    std::weak_ptr<BlockRailTile> m_toBlock;
    Side m_toSide;
    std::vector<std::pair<std::weak_ptr<TurnoutRailTile>, TurnoutPosition>> m_turnouts; //!< required turnout positions for the path
    std::vector<std::pair<std::weak_ptr<DirectionControlRailTile>, DirectionControlState>> m_directionControls; //!< required direction control states for the path
    std::vector<std::pair<std::weak_ptr<CrossRailTile>, CrossState>> m_crossings; //!< required crossing states for the path
    std::vector<std::weak_ptr<SignalRailTile>> m_signals; //!< signals in path
    std::weak_ptr<NXButtonRailTile> m_nxButtonFrom;
    std::weak_ptr<NXButtonRailTile> m_nxButtonTo;

  public:
    static std::vector<std::unique_ptr<BlockPath>> find(BlockRailTile& block);

    BlockPath(BlockRailTile& block, Side side);

    bool hasNXButtons() const
    {
      return !m_nxButtonFrom.expired() && !m_nxButtonTo.expired();
    }

    const BlockRailTile& fromBlock() const
    {
      return m_fromBlock;
    }

    std::shared_ptr<BlockRailTile> toBlock() const
    {
      return m_toBlock.lock();
    }

    std::shared_ptr<NXButtonRailTile> nxButtonFrom() const;
    std::shared_ptr<NXButtonRailTile> nxButtonTo() const;
};

#endif
