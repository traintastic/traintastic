/**
 * server/src/train/trainvehiclelistitem.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Filippo Gentile
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

#include "trainvehiclelistitem.hpp"
#include "trainvehiclelist.hpp"
#include "../world/getworld.hpp"
#include "../core/attributes.hpp"
#include "../core/objectproperty.tpp"
#include "../vehicle/rail/railvehicle.hpp"


TrainVehicleListItem::TrainVehicleListItem(const std::shared_ptr<RailVehicle> &vehicle_, TrainVehicleList &parent, uint32_t itemId)
  : m_parent(parent)
  , m_itemId(itemId)
  , vehicle(this, "vehicle", vehicle_, PropertyFlags::ReadWrite | PropertyFlags::ScriptReadWrite | PropertyFlags::StoreState,
      [this](const std::shared_ptr<RailVehicle>& value) -> bool
      {
        if(!value || m_parent.getItemFromVehicle(value))
          return false; //Vehicle already in train

        if(vehicle)
          disconnectVehicle(*vehicle);

        if(value)
          connectVehicle(*value);

        return true;
      })
  , invertDirection(this, "invert_direction", false, PropertyFlags::ReadWrite | PropertyFlags::ScriptReadWrite | PropertyFlags::StoreState)
{
  auto& world = getWorld(m_parent);

  Attributes::addEnabled(vehicle, true);
  Attributes::addObjectList(vehicle, world.railVehicles);
  m_interfaceItems.add(vehicle);
  Attributes::addEnabled(invertDirection, true);
  m_interfaceItems.add(invertDirection);
}

TrainVehicleListItem::~TrainVehicleListItem()
{
}

std::string TrainVehicleListItem::getObjectId() const
{
  return m_parent.getObjectId().append(".").append(m_parent.items.name()).append(".item").append(std::to_string(m_itemId));
}

void TrainVehicleListItem::destroying()
{
  //NOTE: we cannot normally set vehicle to nullptr (rejected by OnSet callback)
  //So we mirror cleanup operations and manually reset value at end
  if(vehicle)
    disconnectVehicle(*vehicle.value());
  vehicle.setValueInternal(nullptr);

  Object::destroying();
}

void TrainVehicleListItem::save(WorldSaver &saver, nlohmann::json &data, nlohmann::json &state) const
{
  Object::save(saver, data, state);
  data["item_id"] = m_itemId;
}

void TrainVehicleListItem::loaded()
{
  Object::loaded();

  if(vehicle)
      connectVehicle(*vehicle);
}

void TrainVehicleListItem::connectVehicle(RailVehicle &object)
{
  //Connect to new vehicle
  m_vehiclePropertyChanged = object.propertyChanged.connect(
    [this](BaseProperty &prop)
    {
      //Propagate property change
      propertyChanged(prop);
    });

  m_vehicleDestroying = object.onDestroying.connect(
    [this]([[maybe_unused]] Object& obj)
    {
      assert(vehicle.value().get() == &obj);
      vehicle = nullptr;
    });
}

void TrainVehicleListItem::disconnectVehicle(RailVehicle &/*object*/)
{
  //Disconnect from previous vehicle
  m_vehiclePropertyChanged.disconnect();
  m_vehicleDestroying.disconnect();
}
