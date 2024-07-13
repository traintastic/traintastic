/**
 * server/src/board/tile/rail/signal/signalrailtile.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_RAIL_SIGNAL_SIGNALRAILTILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_RAIL_SIGNAL_SIGNALRAILTILE_HPP

#include "../straightrailtile.hpp"
#include <traintastic/enum/autoyesno.hpp>
#include "../../../map/node.hpp"
#include "../../../../core/method.hpp"
#include "../../../../core/event.hpp"
#include "../../../../enum/signalaspect.hpp"
#include "../../../../core/objectproperty.hpp"
#include "../../../../hardware/output/map/signaloutputmap.hpp"

class AbstractSignalPath;
class BlockPath;

class SignalRailTile : public StraightRailTile
{
  DEFAULT_ID("signal")

  protected:
    Node m_node;
    std::unique_ptr<AbstractSignalPath> m_signalPath;
    std::weak_ptr<BlockPath> m_blockPath;

    SignalRailTile(World& world, std::string_view _id, TileId tileId_);

    void destroying() override;
    void addToWorld() override;
    void worldEvent(WorldState state, WorldEvent event) override;

    void boardModified() override;

    virtual bool doSetAspect(SignalAspect value, bool skipAction = false);

    void evaluate();

    void connectOutputMap();

  public:
    static std::optional<OutputActionValue> getDefaultActionValue(SignalAspect signalAspect, OutputType outputType, size_t outputIndex);

    boost::signals2::signal<void (const SignalRailTile&, SignalAspect)> aspectChanged;

    Property<std::string> name;
    Property<AutoYesNo> requireReservation;
    Property<SignalAspect> aspect;
    ObjectProperty<SignalOutputMap> outputMap;
    Method<bool(SignalAspect)> setAspect;
    Event<const std::shared_ptr<SignalRailTile>&, SignalAspect> onAspectChanged;

    ~SignalRailTile() override;

    std::optional<std::reference_wrapper<const Node>> node() const final { return m_node; }
    std::optional<std::reference_wrapper<Node>> node() final { return m_node; }

    bool hasReservedPath() const noexcept;
    std::shared_ptr<BlockPath> reservedPath() const noexcept;

    bool reserve(const std::shared_ptr<BlockPath>& blockPath, bool dryRun = false);
};

#endif
