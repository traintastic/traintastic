/**
 * server/src/zone/zoneblocklist.cpp
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

#include "zoneblocklist.hpp"
#include "blockzonelist.hpp"
#include "../board/list/blockrailtilelisttablemodel.hpp"
#include "../world/getworld.hpp"
#include "../core/attributes.hpp"
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../utils/displayname.hpp"

ZoneBlockList::ZoneBlockList(Zone& zone_, std::string_view parentPropertyName)
  : ObjectList<BlockRailTile>(zone_, parentPropertyName)
  , add{*this, "add",
      [this](const std::shared_ptr<BlockRailTile>& block)
      {
        if(!containsObject(block))
        {
          addObject(block);
          block->zones->add(parent().shared_ptr<Zone>());
        }
      }}
  , remove{*this, "remove",
      [this](const std::shared_ptr<BlockRailTile>& block)
      {
        if(containsObject(block))
        {
          removeObject(block);
          block->zones->remove(parent().shared_ptr<Zone>());
        }
      }}
{
  const auto& world = getWorld(parent());

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, false);
  Attributes::addObjectList(add, world.blockRailTiles);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, false);
  m_interfaceItems.add(remove);
}

TableModelPtr ZoneBlockList::getModel()
{
  return std::make_shared<BlockRailTileListTableModel>(*this);
}

void ZoneBlockList::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  ObjectList<BlockRailTile>::worldEvent(worldState, worldEvent);
  Attributes::setEnabled({add, remove}, contains(worldState, WorldState::Edit) && !contains(worldState, WorldState::Run));
}

bool ZoneBlockList::isListedProperty(std::string_view name)
{
  return BlockRailTileListTableModel::isListedProperty(name);
}

Zone& ZoneBlockList::zone()
{
  return static_cast<Zone&>(parent());
}
