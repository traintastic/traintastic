/**
 * server/src/board/tile/rail/blockrailtile.hpp
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_RAIL_BLOCKRAILTILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_RAIL_BLOCKRAILTILE_HPP

#include "railtile.hpp"
#include <array>
#include <traintastic/enum/blocktraindirection.hpp>
#include "../../map/node.hpp"
#include "../../../core/method.hpp"
#include "../../../core/objectproperty.hpp"
#include "../../../core/vectorproperty.hpp"
#include "../../../enum/blockside.hpp"
#include "../../../enum/blockstate.hpp"
#include "../../../hardware/input/map/blockinputmap.hpp"

class Train;
class TrainBlockStatus;
class BlockInputMapItem;
class BlockPath;

class BlockRailTile : public RailTile
{
  friend class TrainTracking;

  CLASS_ID("board_tile.rail.block")
  DEFAULT_ID("block")
  CREATE_DEF(BlockRailTile)

  private:
    using Paths = std::vector<std::shared_ptr<BlockPath>>;

    Node m_node;
    Paths m_paths; //!< Paths from this block to other block
    Paths m_pathsIn; //!< Paths from other blocks to this block
    std::array<std::weak_ptr<BlockPath>, 2> m_reservedPaths; // index is BlockSide

    std::shared_ptr<TrainBlockStatus> getBlockTrainStatus(const std::shared_ptr<Train>& train);

    void updatePaths();
    void updateHeightWidthMax();

    void fireTrainReserved(const std::shared_ptr<Train>& train, BlockTrainDirection trainDirection);
    void fireTrainEntered(const std::shared_ptr<Train>& train, BlockTrainDirection trainDirection);
    void fireTrainLeft(const std::shared_ptr<Train>& train, BlockTrainDirection trainDirection);

  protected:
    void worldEvent(WorldState worldState, WorldEvent worldEvent) final;
    void loaded() final;
    void destroying() final;
    void setRotate(TileRotate value) final;
    void boardModified() final;

    void updateState();
    void updateTrainMethodEnabled();
    void setState(BlockState value);

  public:
    boost::signals2::signal<void (const BlockRailTile&, BlockState)> stateChanged;

    Property<std::string> name;
    ObjectProperty<BlockInputMap> inputMap;
    Property<BlockState> state;
    VectorProperty<SensorState> sensorStates;
    ObjectVectorProperty<TrainBlockStatus> trains;
    Method<void(std::shared_ptr<Train>)> assignTrain;
    Method<void(std::shared_ptr<Train>)> removeTrain;
    Method<void()> flipTrain;
    Method<bool()> setStateFree;
    Event<const std::shared_ptr<Train>&, const std::shared_ptr<BlockRailTile>&> onTrainAssigned;
    Event<const std::shared_ptr<Train>&, const std::shared_ptr<BlockRailTile>&, BlockTrainDirection> onTrainReserved;
    Event<const std::shared_ptr<Train>&, const std::shared_ptr<BlockRailTile>&, BlockTrainDirection> onTrainEntered;
    Event<const std::shared_ptr<Train>&, const std::shared_ptr<BlockRailTile>&, BlockTrainDirection> onTrainLeft;
    Event<const std::shared_ptr<Train>&, const std::shared_ptr<BlockRailTile>&> onTrainRemoved;

    BlockRailTile(World& world, std::string_view _id);

    std::optional<std::reference_wrapper<const Node>> node() const final { return m_node; }
    std::optional<std::reference_wrapper<Node>> node() final { return m_node; }
    void getConnectors(std::vector<Connector>& connectors) const final;

    const std::vector<std::shared_ptr<BlockPath>>& paths() const
    {
      return m_paths;
    }

    void inputItemValueChanged(BlockInputMapItem& item);
    void identificationEvent(BlockInputMapItem& item, IdentificationEventType eventType, uint16_t identifier, Direction direction, uint8_t category);

    const std::shared_ptr<BlockPath> getReservedPath(BlockSide side) const;
    bool reserve(const std::shared_ptr<BlockPath>& blockPath, const std::shared_ptr<Train>& train, BlockSide side, bool dryRun = false);
    bool release(BlockSide side, bool dryRun = false);

    bool removeTrainInternal(const std::shared_ptr<TrainBlockStatus>& status);
};

#endif
