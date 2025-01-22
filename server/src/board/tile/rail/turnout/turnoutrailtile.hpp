/**
 * server/src/board/tile/rail/turnout/turnoutrailtile.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_RAIL_TURNOUT_TURNOUTRAILTILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_RAIL_TURNOUT_TURNOUTRAILTILE_HPP

#include <chrono>
#include "../railtile.hpp"
#include "../../../map/node.hpp"
#include "../../../../core/objectproperty.hpp"
#include "../../../../core/method.hpp"
#include <traintastic/enum/turnoutposition.hpp>
#include "../../../../hardware/output/map/turnoutoutputmap.hpp"

class BlockPath;

class TurnoutRailTile : public RailTile
{
  DEFAULT_ID("turnout")

  private:
    Node m_node;
    std::weak_ptr<BlockPath> m_reservedPath;

    std::chrono::steady_clock::time_point m_lastRetryStart;
    uint8_t m_retryCount;
    static constexpr uint8_t MAX_RETRYCOUNT = 3;
    static constexpr std::chrono::steady_clock::duration RETRY_DURATION = std::chrono::minutes(1);

  protected:
    TurnoutRailTile(World& world, std::string_view _id, TileId tileId_, size_t connectors);

    void destroying() override;
    void addToWorld() override;
    void worldEvent(WorldState state, WorldEvent event) override;

    bool isValidPosition(TurnoutPosition value);
    virtual bool doSetPosition(TurnoutPosition value, bool skipAction = false);

    void connectOutputMap();

  public:
    boost::signals2::signal<void (const TurnoutRailTile&, TurnoutPosition)> positionChanged;

    Property<std::string> name;
    Property<TurnoutPosition> position;
    ObjectProperty<TurnoutOutputMap> outputMap;
    Method<bool(TurnoutPosition)> setPosition;

    std::optional<std::reference_wrapper<const Node>> node() const final { return m_node; }
    std::optional<std::reference_wrapper<Node>> node() final { return m_node; }

    virtual bool reserve(const std::shared_ptr<BlockPath>& blockPath, TurnoutPosition turnoutPosition, bool dryRun = false);
    bool release(bool dryRun = false);

    TurnoutPosition getReservedPosition() const;
};

#endif
