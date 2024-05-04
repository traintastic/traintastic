/**
 * server/src/board/tile/rail/blockrailtile.cpp
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

#include "blockrailtile.hpp"
#include "../../../core/method.tpp"
#include "../../../core/objectproperty.tpp"
#include "../../../core/objectvectorproperty.tpp"
#include "../../../world/world.hpp"
#include "../../../core/attributes.hpp"
#include "../../../log/log.hpp"
#include "../../../train/train.hpp"
#include "../../../train/trainblockstatus.hpp"
#include "../../../utils/displayname.hpp"
#include "../../map/blockpath.hpp"

constexpr uint8_t toMask(BlockSide side)
{
  return 1 << static_cast<uint8_t>(side);
}

constexpr bool isDirectionTowardsSide(BlockTrainDirection direction, BlockSide side)
{
  return
    (direction == BlockTrainDirection::TowardsA && side == BlockSide::A) ||
    (direction == BlockTrainDirection::TowardsB && side == BlockSide::B);
}

CREATE_IMPL(BlockRailTile)

BlockRailTile::BlockRailTile(World& world, std::string_view _id) :
  RailTile(world, _id, TileId::RailBlock),
  m_node{*this, 2},
  name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store},
  inputMap{this, "input_map", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  state{this, "state", BlockState::Unknown, PropertyFlags::ReadOnly | PropertyFlags::StoreState},
  sensorStates{*this, "sensor_states", {}, PropertyFlags::ReadOnly | PropertyFlags::StoreState}
  , trains{*this, "trains", {}, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
  , assignTrain{*this, "assign_train",
      [this](const std::shared_ptr<Train>& newTrain)
      {
        if(!newTrain) /*[[unlikely]]*/
          return;

        if(trains.empty())
        {
          if(!newTrain->active)
          {
            try
            {
              newTrain->active = true;
            }
            catch(...)
            {
              //! \todo report reason back to caller
            }

            if(!newTrain->active)
              return;
          }

          const auto self = shared_ptr<BlockRailTile>();
          BlockTrainDirection direction = (newTrain->direction == Direction::Reverse) ? BlockTrainDirection::TowardsB : BlockTrainDirection::TowardsA;

          if(!newTrain->blocks.empty())
          {
            //! \todo check if block is connected to the head or tail block of the train
            //! \todo update direction
            return; // not yet supported
          }

          auto status = TrainBlockStatus::create(*this, *newTrain, direction);
          newTrain->blocks.appendInternal(status);
          trains.appendInternal(status);

          updateTrainMethodEnabled();
          if(state == BlockState::Free || state == BlockState::Unknown)
            updateState();
          Log::log(*this, LogMessage::N3001_ASSIGNED_TRAIN_X_TO_BLOCK_X, newTrain->name.value(), name.value());

          if(m_world.simulation)
          {
            // TODO: use train length if block has multiple occupancy sensors, center train in block
            for(const auto& item : *inputMap)
            {
              if(item->input && item->input->interface)
              {
                if(item->type == SensorType::OccupancyDetector)
                  item->input->simulateChange(item->invert.value() ? SimulateInputAction::SetFalse : SimulateInputAction::SetTrue);
                else
                  assert(false); // not yet implemented
              }
            }
          }

          fireEvent(onTrainAssigned, newTrain, self);
        }
      }}
  , removeTrain{*this, "remove_train",
      [this]()
      {
        if(trains.size() == 1 && trains[0]->train->isStopped)
        {
          const auto self = shared_ptr<BlockRailTile>();
          auto oldTrain = trains[0]->train.value();
          if(!oldTrain->blocks.empty() && self != (**oldTrain->blocks.begin()).block.value()
              && self != (**oldTrain->blocks.rbegin()).block.value())
            return; // only possible to remove the train from the head or tail block

          removeOneTrain(oldTrain);
        }
      }}
  , flipTrain{*this, "flip_train",
      [this]()
      {
        if(trains.size() == 1)
        {
          assert(trains[0]->train);
          const auto& train = *trains[0]->train;
          if(train.isStopped && train.blocks.size() == 1 && train.blocks[0]->block.operator->() == this)
          {
            trains[0]->direction.setValueInternal(!trains[0]->direction.value());
          }
        }
      }}
  , setStateFree{*this, "set_state_free",
      [this]()
      {
        if(trains.empty()) // No train may be assigned to or have reserved the block.
        {
          // Check sensor states, they may not be occupied:
          if(sensorStates.empty())
          {
            setState(BlockState::Free);
            return true;
          }
          // TODO: set sensors to free if none is occupied
        }
        return false;
      }}
  , onTrainAssigned{*this, "on_train_assigned", EventFlags::Scriptable}
  , onTrainReserved{*this, "on_train_reserved", EventFlags::Scriptable}
  , onTrainEntered{*this, "on_train_entered", EventFlags::Scriptable}
  , onTrainLeft{*this, "on_train_left", EventFlags::Scriptable}
  , onTrainRemoved{*this, "on_train_removed", EventFlags::Scriptable}
{
  inputMap.setValueInternal(std::make_shared<BlockInputMap>(*this, inputMap.name()));

  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  m_interfaceItems.add(inputMap);

  Attributes::addObjectEditor(state, false);
  Attributes::addValues(state, blockStateValues);
  m_interfaceItems.add(state);

  Attributes::addObjectEditor(sensorStates, false);
  Attributes::addValues(sensorStates, sensorStateValues);
  m_interfaceItems.add(sensorStates);

  Attributes::addObjectEditor(trains, false);
  m_interfaceItems.add(trains);

  Attributes::addEnabled(assignTrain, true);
  Attributes::addObjectEditor(assignTrain, false);
  Attributes::addObjectList(assignTrain, world.trains);
  m_interfaceItems.add(assignTrain);

  Attributes::addEnabled(removeTrain, false);
  Attributes::addObjectEditor(removeTrain, false);
  m_interfaceItems.add(removeTrain);

  Attributes::addEnabled(flipTrain, false);
  Attributes::addObjectEditor(flipTrain, false);
  m_interfaceItems.add(flipTrain);

  m_interfaceItems.add(onTrainAssigned);
  m_interfaceItems.add(onTrainReserved);
  m_interfaceItems.add(onTrainEntered);
  m_interfaceItems.add(onTrainLeft);
  m_interfaceItems.add(onTrainRemoved);

  updateHeightWidthMax();
}

void BlockRailTile::inputItemValueChanged(BlockInputMapItem& item)
{
  if(item.value() == SensorState::Occupied)
  {
    switch(state.value())
    {
      case BlockState::Free:
      case BlockState::Unknown:
      {
        // Something entered the block, try to determine what it is.

        if(inputMap->items.size() > 2 && (&item != inputMap->items.front().get()) && (&item != inputMap->items.back().get()))
        {
          // Non block edge sensor.

          //! \todo log something (at least in debug)

          break;
        }

        const bool anySide = inputMap->items.size() == 1; // block with one sensor
        const BlockSide enterSide = (&item == inputMap->items.front().get()) ? BlockSide::A : BlockSide::B;

        std::shared_ptr<Train> train;
        BlockTrainDirection direction = BlockTrainDirection::Unknown;

        for(const auto& path : m_pathsIn)
        {
          if(path->toBlock().get() == this && (anySide || path->toSide() == enterSide) && !path->fromBlock().trains.empty() && path->isReady())
          {
            const auto status = path->fromSide() == BlockSide::A ? path->fromBlock().trains.front() : path->fromBlock().trains.back();

            if(isDirectionTowardsSide(status->direction, path->fromSide()) &&
                status->train &&
                status->train->powered &&
                status->train->mode == TrainMode::ManualUnprotected &&
                !status->train->isStopped)
            {
              if(train) // another train?? then we don't know
              {
                train.reset();
                //! \todo log something (at least in debug)
                break;
              }
              else
              {
                train = status->train.value();
                direction = path->toSide() == BlockSide::A ? BlockTrainDirection::TowardsB : BlockTrainDirection::TowardsA;
              }
            }
          }
        }

        if(train)
        {
          auto blockStatus = TrainBlockStatus::create(*this, *train, direction);

          blockStatus->train->blocks.insertInternal(0, blockStatus); // head of train
          trains.appendInternal(blockStatus);
          updateTrainMethodEnabled();

          fireEvent(
            onTrainEntered,
            blockStatus->train.value(),
            shared_ptr<BlockRailTile>(),
            blockStatus->direction.value());
        }
        break;
      }
      case BlockState::Reserved:
      {
        const bool inputA = (inputMap->items.front().get() == &item);
        const auto pathA = inputA ? m_reservedPaths[0].lock() : nullptr;
        const bool enterA = pathA && pathA->toBlock().get() == this;

        const bool inputB = (inputMap->items.back().get() == &item);
        const auto pathB = inputB ? m_reservedPaths[1].lock() : nullptr;
        const bool enterB = pathB && pathB->toBlock().get() == this;

        if(enterA != enterB)
        {
          auto& blockStatus = enterA ? trains.front() : trains.back();
          blockStatus->train->blocks.insertInternal(0, blockStatus);

          fireEvent(
            onTrainEntered,
            blockStatus->train.value(),
            shared_ptr<BlockRailTile>(),
            blockStatus->direction.value());
        }
        else
        {
          // assignment or something is wrong
        }
        break;
      }
      case BlockState::Occupied:
        break;
    }
  }

  if(inputMap->items.size() != sensorStates.size())
  {
    std::vector<SensorState> values;
    values.reserve(inputMap->items.size());
    for(const auto& v : inputMap->items)
      values.emplace_back(v->value());
    sensorStates.setValuesInternal(values);
  }
  else
    sensorStates.setValueInternal(inputMap->items.indexOf(item), item.value());

  updateState();

  if(item.value() == SensorState::Free && state.value() == BlockState::Reserved)
  {
    if(trains.size() == 1)
    {
      auto blockStatus = trains.front();

      // Train must be in at least two blocks, else we loose it.
      // Release tailing block of train only. (When using current detection not all wagons might consume power.)
      if(blockStatus->train &&
          blockStatus->train->blocks.size() > 1 &&
          blockStatus->train->blocks.back() == blockStatus)
      {
        blockStatus->train->blocks.removeInternal(blockStatus);
        trains.removeInternal(blockStatus);
        updateTrainMethodEnabled();

        updateState();

        fireEvent(
          onTrainLeft,
          blockStatus->train.value(),
          shared_ptr<BlockRailTile>(),
          blockStatus->direction.value());

        const auto pathA = m_reservedPaths[0].lock();
        const bool exitA = pathA && &pathA->fromBlock() == this;

        const auto pathB = m_reservedPaths[1].lock();
        const bool exitB = pathB && &pathB->fromBlock() == this;

        if(blockStatus->direction.value() == BlockTrainDirection::TowardsA && exitA)
        {
          pathA->release();
        }
        else if(blockStatus->direction.value() == BlockTrainDirection::TowardsB && exitB)
        {
          pathB->release();
        }

        blockStatus->destroy();
#ifndef NDEBUG
        std::weak_ptr<TrainBlockStatus> blockStatusWeak = blockStatus;
        blockStatus.reset();
        assert(blockStatusWeak.expired());
#endif
      }
    }
  }
}

void BlockRailTile::identificationEvent(BlockInputMapItem& /*item*/, IdentificationEventType eventType, uint16_t identifier, Direction direction, uint8_t /*category*/)
{
  const auto self = shared_ptr<BlockRailTile>();
  BlockTrainDirection blockDirection = BlockTrainDirection::Unknown;
  if(direction != Direction::Unknown)
    blockDirection = (direction == Direction::Reverse) ? BlockTrainDirection::TowardsB : BlockTrainDirection::TowardsA;

  if(trains.empty())
  {
    switch(eventType)
    {
      case IdentificationEventType::Absent:
        break; // nothing to do...its gone

      case IdentificationEventType::Present:
        //!< \todo assign train (if allowed and possible)
        trains.appendInternal(TrainBlockStatus::create(*this, std::string("#").append(std::to_string(identifier)), blockDirection));
        updateTrainMethodEnabled();
        if(state == BlockState::Free || state == BlockState::Unknown)
          updateState();
        break;

      case IdentificationEventType::Seen:
        break;
    }
  }
  else
  {
    switch(eventType)
    {
      case IdentificationEventType::Absent:
      {
        const auto identification = std::string("#").append(std::to_string(identifier));
        for(const auto& train : trains)
        {
          if(train->identification.value() == identification)
          {
            train->destroy();
            updateState();
            break;
          }
        }
        break;
      }
      case IdentificationEventType::Present:
        break;

      case IdentificationEventType::Seen:
        break;
    }
  }
}

const std::shared_ptr<BlockPath> BlockRailTile::getReservedPath(BlockSide side) const
{
  assert(side == BlockSide::A || side == BlockSide::B);
  return m_reservedPaths[static_cast<uint8_t>(side)].lock();
}

bool BlockRailTile::reserve(const std::shared_ptr<BlockPath>& blockPath, const std::shared_ptr<Train>& train, BlockSide side, bool dryRun)
{
  const uint8_t mask = toMask(side);

  if(state == BlockState::Unknown)
  {
    return false; // can't reserve block with unknown state
  }

  if((reservedState() & mask))
  {
    return false; // already reserved
  }

  if(state != BlockState::Free)
  {
    if(trains.empty())
    {
      return false; // not free block, but no train
    }

    const auto& status = (side == BlockSide::A) ? *trains.front() : *trains.back();
    if(!status.train || status.train.value() != train)
    {
      return false; // no train or other train assigned to block
    }

    if((side == BlockSide::A && status.direction != BlockTrainDirection::TowardsA) ||
        (side == BlockSide::B && status.direction != BlockTrainDirection::TowardsB))
    {
      //! \todo allow direction change, add block property, automatic for dead ends
      return false; // invalid train direction
    }
  }

  if(!dryRun)
  {
    m_reservedPaths[static_cast<uint8_t>(side)] = blockPath;
    RailTile::setReservedState(reservedState() | mask);
    if(state == BlockState::Free)
    {
      const auto direction = side == BlockSide::A ? BlockTrainDirection::TowardsB : BlockTrainDirection::TowardsA;
      trains.appendInternal(TrainBlockStatus::create(*this, *train, direction));
      updateTrainMethodEnabled();
      fireEvent(onTrainReserved, train, shared_ptr<BlockRailTile>(), direction);
    }
    updateState();
  }

  return true;
}

bool BlockRailTile::release(BlockSide side, bool dryRun)
{
  if(!dryRun)
  {
    m_reservedPaths[static_cast<uint8_t>(side)].reset();
    RailTile::setReservedState(reservedState() & ~toMask(side));
  }
  return true;
}

bool BlockRailTile::removeOneTrain(const std::shared_ptr<Train> &oldTrain)
{
  if(!oldTrain)
    return false;

  const auto self = shared_ptr<BlockRailTile>();

  const auto it = std::find_if(trains.begin(), trains.end(),
    [&oldTrain](auto& status)
    {
      return status->train.value() == oldTrain;
    });

  if(it == trains.end()) /*[[unlikely]]*/
    return false; // can't remove a train that isn't in the block

  (**it).destroy();

  updateTrainMethodEnabled();
  if(state == BlockState::Reserved)
    updateState();
  Log::log(*this, LogMessage::N3002_REMOVED_TRAIN_X_FROM_BLOCK_X, oldTrain->name.value(), name.value());

  if(m_world.simulation && trains.empty())
  {
    for(const auto& item : *inputMap)
    {
      if(item->input && item->input->interface)
      {
        if(item->type == SensorType::OccupancyDetector)
          item->input->simulateChange(item->invert.value() ? SimulateInputAction::SetTrue : SimulateInputAction::SetFalse);
        else
          assert(false); // not yet implemented
      }
    }
  }

  fireEvent(onTrainRemoved, oldTrain, self);

  return true;
}

void BlockRailTile::updateState()
{
  if(!inputMap->items.empty())
  {
    bool allFree = true;

    for(const auto& item : inputMap->items)
    {
      if(item->value() == SensorState::Occupied)
      {
        setState(BlockState::Occupied);
        return;
      }
      if(item->value() != SensorState::Free)
      {
        allFree = false;
        break;
      }
    }

    if(allFree)
    {
      setState(trains.empty() ? BlockState::Free : BlockState::Reserved);
      return;
    }
  }

  setState(trains.empty() ? BlockState::Unknown : BlockState::Reserved);
}

void BlockRailTile::updateTrainMethodEnabled()
{
  Attributes::setEnabled(assignTrain, trains.empty());
  Attributes::setEnabled(removeTrain, trains.size() == 1);
  Attributes::setEnabled(flipTrain, trains.size() == 1);
}

void BlockRailTile::setState(BlockState value)
{
  if(value == state)
    return;

  state.setValueInternal(value);
  stateChanged(*this, value);
}

void BlockRailTile::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  RailTile::worldEvent(worldState, worldEvent);

  const bool editable = contains(worldState, WorldState::Edit);

  Attributes::setEnabled(name, editable);
}

void BlockRailTile::loaded()
{
  RailTile::loaded();
  updateTrainMethodEnabled();
  updateHeightWidthMax();
}

void BlockRailTile::destroying()
{
  const auto self = shared_ptr<BlockRailTile>();
  while(!trains.empty())
  {
    trains.back()->destroy();
  }
  RailTile::destroying();
}

void BlockRailTile::getConnectors(std::vector<Connector>& connectors) const
{
  if(rotate == TileRotate::Deg0)
  {
    connectors.emplace_back(location(), Connector::Direction::North, Connector::Type::Rail);
    connectors.emplace_back(location().adjusted(0, height - 1), Connector::Direction::South, Connector::Type::Rail);
  }
  else if(rotate == TileRotate::Deg90)
  {
    connectors.emplace_back(location(), Connector::Direction::West, Connector::Type::Rail);
    connectors.emplace_back(location().adjusted(width - 1, 0), Connector::Direction::East, Connector::Type::Rail);
  }
  else
    assert(false);
}

void BlockRailTile::setRotate(TileRotate value)
{
  if(value == rotate)
    return;

  if(value == TileRotate::Deg0 || value == TileRotate::Deg90)
  {
    RailTile::setRotate(value);

    uint8_t tmp = height;
    height.setValueInternal(width);
    width.setValueInternal(tmp);

    updateHeightWidthMax();
  }
}

void BlockRailTile::boardModified()
{
  updatePaths(); //! \todo improvement: only update if a connected tile is changed
}

void BlockRailTile::updatePaths()
{
  auto current = std::move(m_paths);
  m_paths.clear(); // make sure it is empty, it problably is after the move
  auto found = BlockPath::find(*this);

  while(!current.empty()) // handle existing paths
  {
    auto it = std::find_if(found.begin(), found.end(),
      [&currentPath=*current.front()](const auto& foundPath)
      {
        return currentPath == *foundPath;
      });

    if(it != found.end())
    {
      found.erase(it);
      m_paths.emplace_back(std::move(current.front()));
    }
    current.erase(current.begin());
  }

  for(auto& path : current) // no longer existing paths
  {
    auto& pathsIn = path->toBlock()->m_pathsIn;
    if(auto it = std::find(pathsIn.begin(), pathsIn.end(), path); it != pathsIn.end())
    {
      pathsIn.erase(it);
    }
  }

  for(auto& path : found) // new paths
  {
    path->toBlock()->m_pathsIn.emplace_back(path);
    m_paths.emplace_back(std::move(path));
  }
}

void BlockRailTile::updateHeightWidthMax()
{
    const bool vertical = (rotate == TileRotate::Deg0);
    Attributes::setMax<uint8_t>(height, vertical ? TileData::heightMax : 1);
    Attributes::setMax<uint8_t>(width, !vertical ? TileData::widthMax : 1);
}
