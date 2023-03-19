/**
 * server/src/train/trainvehiclelist.cpp
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

#include "trainvehiclelist.hpp"
#include "../vehicle/rail/railvehiclelisttablemodel.hpp"
#include "train.hpp"
#include "../world/getworld.hpp"
#include "../world/world.hpp"
#include "../core/attributes.hpp"
#include "../utils/displayname.hpp"

TrainVehicleList::TrainVehicleList(Train& train_, std::string_view parentPropertyName)
  : ObjectList<RailVehicle>(train_, parentPropertyName)
  , add{*this, "add",
      [this](const std::shared_ptr<RailVehicle>& vehicle)
      {
        if(!containsObject(vehicle))
        {
          addObject(vehicle);
          train().vehiclesChanged();
        }
      }}
  , remove{*this, "remove",
      [this](const std::shared_ptr<RailVehicle>& vehicle)
      {
        removeObject(vehicle);
        train().vehiclesChanged();
      }}
  , move{*this, "move",
      [this](uint32_t from, uint32_t to)
      {
        if(from >= length || to >= length || from == to)
          return;

        if(from > to)
        {
          std::rotate(m_items.rend() - from - 1, m_items.rend() - from, m_items.rend() - to);
          rowsChanged(to, from);
        }
        else
        {
          std::rotate(m_items.begin() + from, m_items.begin() + from + 1, m_items.begin() + to + 1);
          rowsChanged(from, to);
        }
      }}
  , reverse{*this, "reverse",
      [this]()
      {
        if(m_items.empty())
          return;
        std::reverse(m_items.begin(), m_items.end());
        rowsChanged(0, length - 1);
      }}
{
  const auto& world = getWorld(parent());

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, false);
  Attributes::addObjectList(add, world.railVehicles);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, false);
  m_interfaceItems.add(remove);

  Attributes::addEnabled(move, false);
  m_interfaceItems.add(move);

  Attributes::addEnabled(reverse, false);
  m_interfaceItems.add(reverse);
}

TableModelPtr TrainVehicleList::getModel()
{
  return std::make_shared<RailVehicleListTableModel>(*this);
}

bool TrainVehicleList::isListedProperty(std::string_view name)
{
  return RailVehicleListTableModel::isListedProperty(name);
}

void TrainVehicleList::propertyChanged(BaseProperty& property)
{
  if(property.name() == "lob")
    train().updateLength();
  else if(property.name() == "total_weight")
    train().updateWeight();
  else if(property.name() == "speed_max")
    train().updateSpeedMax();
}

Train& TrainVehicleList::train()
{
  return static_cast<Train&>(parent());
}
