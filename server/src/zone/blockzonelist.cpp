/**
 * server/src/zone/blockzonelist.cpp
 *
 * This file is part of the zonetastic source code.
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

#include "blockzonelist.hpp"
#include "zoneblocklist.hpp"
#include "zonelisttablemodel.hpp"
#include "zone.hpp"
#include "../world/getworld.hpp"
#include "../world/world.hpp"
#include "../core/attributes.hpp"
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../utils/displayname.hpp"

BlockZoneList::BlockZoneList(BlockRailTile& block_, std::string_view parentPropertyName)
  : ObjectList<Zone>(block_, parentPropertyName)
  , add{*this, "add",
      [this](const std::shared_ptr<Zone>& zone)
      {
        if(!containsObject(zone))
        {
          addObject(zone);
          zone->blocks->add(parent().shared_ptr<BlockRailTile>());
        }
      }}
  , remove{*this, "remove",
      [this](const std::shared_ptr<Zone>& zone)
      {
        if(containsObject(zone))
        {
          removeObject(zone);
          zone->blocks->remove(parent().shared_ptr<BlockRailTile>());
        }
      }}
{
  const auto& world = getWorld(parent());

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, false);
  Attributes::addObjectList(add, world.zones);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, false);
  m_interfaceItems.add(remove);
}

TableModelPtr BlockZoneList::getModel()
{
  return std::make_shared<ZoneListTableModel>(*this);
}

void BlockZoneList::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  ObjectList<Zone>::worldEvent(worldState, worldEvent);
  Attributes::setEnabled({add, remove}, contains(worldState, WorldState::Edit) && !contains(worldState, WorldState::Run));
}

bool BlockZoneList::isListedProperty(std::string_view name)
{
  return ZoneListTableModel::isListedProperty(name);
}

BlockRailTile& BlockZoneList::block()
{
  return static_cast<BlockRailTile&>(parent());
}
