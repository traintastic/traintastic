/**
 * server/src/hardware/identification/list/identificationlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#include "identificationlist.hpp"
#include "identificationlisttablemodel.hpp"
#include "../identificationcontroller.hpp"
#include "../../../world/getworld.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"

IdentificationList::IdentificationList(Object& _parent, std::string_view parentPropertyName, IdentificationListColumn _columns)
  : ObjectList<Identification>(_parent, parentPropertyName)
  , columns{_columns}
  , add{*this, "add",
      [this]()
      {
        auto& world = getWorld(parent());
        auto identification = Identification::create(world, world.getUniqueId(Identification::defaultId));
        if(const auto controller = std::dynamic_pointer_cast<IdentificationController>(parent().shared_from_this()))
          identification->interface = controller;
        return identification;
      }}
  , remove{*this, "remove", std::bind(&IdentificationList::removeMethodHandler, this, std::placeholders::_1)}
/*
  , identificationMonitor{*this, "identification_monitor",
      [this]()
      {
        if(const auto controller = std::dynamic_pointer_cast<IdentificationController>(parent().shared_from_this()))
          return controller->identificationMonitor(IdentificationController::defaultIdentificationChannel);
        return std::shared_ptr<IdentificationMonitor>();
      }}
  , identificationMonitorChannel{*this, "identification_monitor_channel",
      [this](uint32_t channel)
      {
        if(const auto controller = std::dynamic_pointer_cast<IdentificationController>(parent().shared_from_this()))
          return controller->identificationMonitor(channel);
        return std::shared_ptr<IdentificationMonitor>();
      }}
*/
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, editable);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);
/*
  if(auto* controller = dynamic_cast<IdentificationController*>(&_parent))
  {
    const auto* channels = controller->identificationChannels();
    if(channels && !channels->empty())
    {
      Attributes::addDisplayName(identificationMonitorChannel, DisplayName::Hardware::identificationMonitor);
      Attributes::addValues(identificationMonitorChannel, channels);
      Attributes::addAliases(identificationMonitorChannel, channels, controller->identificationChannelNames());
      m_interfaceItems.add(identificationMonitorChannel);
    }
    else
    {
      Attributes::addDisplayName(identificationMonitor, DisplayName::Hardware::identificationMonitor);
      m_interfaceItems.add(identificationMonitor);
    }
  }
*/
}

TableModelPtr IdentificationList::getModel()
{
  return std::make_shared<IdentificationListTableModel>(*this);
}

void IdentificationList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Identification>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(add, editable);
  Attributes::setEnabled(remove, editable);
}

bool IdentificationList::isListedProperty(std::string_view name)
{
  return IdentificationListTableModel::isListedProperty(name);
}
