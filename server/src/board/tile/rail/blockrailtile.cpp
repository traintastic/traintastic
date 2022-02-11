/**
 * server/src/board/tile/rail/blockrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2021 Reinder Feenstra
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
#include "../../../world/world.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"

BlockRailTile::BlockRailTile(World& world, std::string_view _id) :
  RailTile(world, _id, TileId::RailBlock),
  name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store},
  inputMap{this, "input_map", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  state{this, "state", BlockState::Unknown, PropertyFlags::ReadOnly | PropertyFlags::StoreState},
  sensorStates{*this, "sensor_states", {}, PropertyFlags::ReadOnly | PropertyFlags::StoreState}
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
        state.setValueInternal(BlockState::Occupied);
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
      state.setValueInternal(BlockState::Free);
      return;
    }
  }

  state.setValueInternal(BlockState::Unknown);
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
  updateHeightWidthMax();
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
