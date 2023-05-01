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
#include "trainvehiclelistitem.hpp"
#include "trainvehiclelisttablemodel.hpp"
#include "train.hpp"
#include "../vehicle/rail/railvehicle.hpp"
#include "../world/getworld.hpp"
#include "../world/world.hpp"
#include "../core/attributes.hpp"
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../core/objectvectorproperty.tpp"
#include "../utils/displayname.hpp"

TrainVehicleList::TrainVehicleList(Train& train_, std::string_view parentPropertyName)
  : SubObject(train_, parentPropertyName)
  , items{*this, "items", {}, PropertyFlags::ReadOnly | PropertyFlags::ScriptReadOnly | PropertyFlags::StoreState | PropertyFlags::SubObject}
  , add{*this, "add",
      [this](const std::shared_ptr<RailVehicle>& vehicle)
      {
        addObject(vehicle);
      }}
  , remove{*this, "remove",
      [this](const std::shared_ptr<TrainVehicleListItem>& item)
      {
        removeObject(item);
      }}
  , move{*this, "move",
      [this](uint32_t from, uint32_t to)
      {
        if(from >= items.size() || to >= items.size() || from == to)
          return;

        auto fromVal = items[from];
        items.moveInternal(fromVal, to - from);

        if(from > to)
        {
          rowsChanged(to, from);
        }
        else
        {
          rowsChanged(from, to);
        }
      }}
  , reverse{*this, "reverse",
      [this]()
      {
        if(items.empty())
          return;
        items.reverseInternal();
        rowsChanged(0, items.size() - 1);
      }}
{
  const auto& world = getWorld(parent());

  m_interfaceItems.add(items);

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

void TrainVehicleList::load(WorldLoader& loader, const nlohmann::json& data)
{
  nlohmann::json objects = data.value("items", nlohmann::json::array());
  if(!objects.empty())
  {
    std::vector<std::shared_ptr<TrainVehicleListItem>> values;
    for(const auto& object : objects.items())
    {
      nlohmann::json itemId = object.value().value("item_id", nlohmann::json());
      if(itemId.is_number_unsigned())
      {
        auto item = std::make_shared<TrainVehicleListItem>(*this, itemId.get<uint32_t>());
        m_propertyChanged.emplace(item.get(), item->propertyChanged.connect(std::bind(&TrainVehicleList::propertyChanged, this, std::placeholders::_1)));
        values.emplace_back(item);
      }
      else
        break;
      }
      items.load(std::move(values));
  }
  SubObject::load(loader, data);
  rowCountChanged();
}

void TrainVehicleList::propertyChanged(BaseProperty &property)
{
  if(property.name() == "lob")
    train().updateLength();
  else if(property.name() == "total_weight")
    train().updateWeight();
  else if(property.name() == "speed_max")
    train().updateSpeedMax();

  if(!m_models.empty() && TrainVehicleListTableModel::isListedProperty(property.name()))
  {
    ObjectPtr obj = property.object().shared_from_this();
    const uint32_t rows = static_cast<uint32_t>(items.size());
    for(uint32_t row = 0; row < rows; row++)
    {
      if(items[row] == obj)
      {
        for(auto& model : m_models)
        {
          model->propertyChanged(property, row);
          break;
        }
      }
    }
  }
}

TableModelPtr TrainVehicleList::getModel()
{
  return std::make_shared<TrainVehicleListTableModel>(*this);
}

void TrainVehicleList::rowCountChanged()
{
  const auto size = items.size();
  //length.setValueInternal(static_cast<uint32_t>(size));
  for(auto& model : m_models)
    model->setRowCount(static_cast<uint32_t>(size));
}

void TrainVehicleList::rowsChanged(uint32_t first, uint32_t last)
{
  for(auto& model : m_models)
  {
    model->rowsChanged(first, last);
  }
}

void TrainVehicleList::addObject(std::shared_ptr<RailVehicle> vehicle)
{
  std::shared_ptr<TrainVehicleListItem> object = getItemFromVehicle(vehicle);
  if(object)
    return; //Vehicle is already in this train

  object = std::make_shared<TrainVehicleListItem>(*this, getItemId());
  object->vehicle.setValueInternal(vehicle);

  items.appendInternal(object);
  m_propertyChanged.emplace(object.get(), object->propertyChanged.connect(std::bind(&TrainVehicleList::propertyChanged, this, std::placeholders::_1)));

  rowCountChanged();
  train().vehiclesChanged();
}

void TrainVehicleList::removeObject(const std::shared_ptr<TrainVehicleListItem> &item)
{
  auto it = std::find(items.begin(), items.end(), item);
  if(it == items.end())
    return;

  m_propertyChanged[item.get()].disconnect();
  m_propertyChanged.erase(item.get());

  item->destroy();
  items.removeInternal(item);

  rowCountChanged();

  uint32_t row = std::distance(items.begin(), it);
  for(auto& model : m_models)
  {
    model->rowRemovedHack(row);
  }

  train().vehiclesChanged();
}

Train& TrainVehicleList::train()
{
  return static_cast<Train&>(parent());
}

uint32_t TrainVehicleList::getItemId() const
{
  uint32_t itemId = 1;
  while(std::find_if(begin(), end(), [itemId](const auto& it){ return it->itemId() == itemId; }) != end())
    itemId++;
  return itemId;
}

std::shared_ptr<TrainVehicleListItem> TrainVehicleList::getItemFromVehicle(const std::shared_ptr<RailVehicle>& vehicle) const
{
  static const std::shared_ptr<TrainVehicleListItem> null;

  auto it = std::find_if(begin(), end(),
    [vehicle](const std::shared_ptr<TrainVehicleListItem>& item)
    {
      return item->vehicle.value() == vehicle;
    });

  return it == end() ? null : *it;
}
