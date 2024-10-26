/**
 * server/src/zone/zonelist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "zone.hpp"
#include "zonelist.hpp"
#include "zonelisttablemodel.hpp"
#include "../world/world.hpp"
#include "../world/getworld.hpp"
#include "../core/attributes.hpp"
#include "../core/method.tpp"
#include "../utils/displayname.hpp"

ZoneList::ZoneList(Object& _parent, std::string_view parentPropertyName)
  : ObjectList<Zone>(_parent, parentPropertyName)
  , create{*this, "create",
      [this]()
      {
        auto& world = getWorld(parent());
        return Zone::create(world, world.getUniqueId(Zone::defaultId));
      }}
  , delete_{*this, "delete",
      [this](const std::shared_ptr<Zone>& zone)
      {
        deleteMethodHandler(zone);
      }}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(create, DisplayName::List::create);
  Attributes::addEnabled(create, editable);
  m_interfaceItems.add(create);

  Attributes::addDisplayName(delete_, DisplayName::List::delete_);
  Attributes::addEnabled(delete_, editable);
  m_interfaceItems.add(delete_);
}

TableModelPtr ZoneList::getModel()
{
  return std::make_shared<ZoneListTableModel>(*this);
}

void ZoneList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Zone>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(create, editable);
  Attributes::setEnabled(delete_, editable);
}

bool ZoneList::isListedProperty(std::string_view name)
{
  return ZoneListTableModel::isListedProperty(name);
}
