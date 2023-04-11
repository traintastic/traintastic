/**
 * server/src/board/tile/rail/blockrailtile.cpp
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

#include "blockrailtile.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../world/world.hpp"
#include "../../../core/attributes.hpp"
#include "../../../log/log.hpp"
#include "../../../utils/displayname.hpp"

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
        if(trains.empty())
        {
          trains.appendInternal(newTrain);
          updateTrainMethodEnabled();
          if(state == BlockState::Free || state == BlockState::Unknown)
            updateState();
          Log::log(*this, LogMessage::N3001_ASSIGNED_TRAIN_X_TO_BLOCK_X, newTrain->name.value(), name.value());

          if(m_world.simulation)
          {
            // TODO: use train length if block has multiple occupy sensors, center train in block
            for(const auto& item : *inputMap)
            {
              if(item->input && item->input->interface)
              {
                if(item->type == SensorType::OccupyDetector)
                  item->input->simulateChange(item->invert.value() ? SimulateInputAction::SetFalse : SimulateInputAction::SetTrue);
                else
                  assert(false); // not yet implemented
              }
            }
          }

          fireEvent<const std::shared_ptr<Train>&, const std::shared_ptr<BlockRailTile>&>(onTrainAssigned, newTrain, shared_ptr<BlockRailTile>());
        }
      }}
  , removeTrain{*this, "remove_train",
      [this]()
      {
        if(trains.size() == 1 && trains[0]->isStopped)
        {
          auto oldTrain = trains[0];
          trains.clearInternal();
          updateTrainMethodEnabled();
          if(state == BlockState::Reserved)
            updateState();
          Log::log(*this, LogMessage::N3002_REMOVED_TRAIN_X_FROM_BLOCK_X, oldTrain->name.value(), name.value());

          if(m_world.simulation)
          {
            for(const auto& item : *inputMap)
            {
              if(item->input && item->input->interface)
              {
                if(item->type == SensorType::OccupyDetector)
                  item->input->simulateChange(item->invert.value() ? SimulateInputAction::SetTrue : SimulateInputAction::SetFalse);
                else
                  assert(false); // not yet implemented
              }
            }
          }

          fireEvent<const std::shared_ptr<Train>&, const std::shared_ptr<BlockRailTile>&>(onTrainRemoved, oldTrain, shared_ptr<BlockRailTile>());
        }
      }}
  , onTrainAssigned{*this, "on_train_assigned", EventFlags::Scriptable}
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

  m_interfaceItems.add(onTrainAssigned);
  m_interfaceItems.add(onTrainRemoved);

  updateHeightWidthMax();
}

void BlockRailTile::inputItemValueChanged(BlockInputMapItem& item)
{
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

void BlockRailTile::updateHeightWidthMax()
{
    const bool vertical = (rotate == TileRotate::Deg0);
    Attributes::setMax<uint8_t>(height, vertical ? TileData::heightMax : 1);
    Attributes::setMax<uint8_t>(width, !vertical ? TileData::widthMax : 1);
}
