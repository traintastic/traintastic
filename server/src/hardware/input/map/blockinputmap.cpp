/**
 * server/src/hardware/input/map/blockinputmap.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "blockinputmap.hpp"
#include "../../../world/getworld.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"

BlockInputMap::BlockInputMap(Object& _parent, const std::string& parentPropertyName) :
  InputMap(_parent, parentPropertyName),
  items{*this, "items", {}, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  add{*this, "add",
    [this]()
    {
      items.appendInternal(std::make_shared<BlockInputMapItem>(*this, getItemId()));
    }},
  remove{*this, "remove",
    [this](const std::shared_ptr<BlockInputMapItem>& item)
    {
      if(!item)
        return;
      item->destroy();
      items.removeInternal(item);
    }},
  moveUp{*this, "move_up",
    [this](const std::shared_ptr<BlockInputMapItem>& item)
    {
      items.moveInternal(item, -1);
    }},
  moveDown{*this, "move_down",
    [this](const std::shared_ptr<BlockInputMapItem>& item)
    {
      items.moveInternal(item, +1);
    }}
{
  auto w = getWorld(&_parent);
  assert(w);

  const bool editable = contains(w->state.value(), WorldState::Edit) && !contains(w->state.value(), WorldState::Run);

  m_interfaceItems.add(items);

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, editable);
  Attributes::addObjectList(add, w->inputs);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);

  Attributes::addDisplayName(moveUp, DisplayName::List::moveUp);
  Attributes::addEnabled(moveUp, editable);
  m_interfaceItems.add(moveUp);

  Attributes::addDisplayName(moveDown, DisplayName::List::moveDown);
  Attributes::addEnabled(moveDown, editable);
  m_interfaceItems.add(moveDown);
}

void BlockInputMap::load(WorldLoader& loader, const nlohmann::json& data)
{
  nlohmann::json objects = data.value("items", nlohmann::json::array());
  if(!objects.empty())
  {
    std::vector<std::shared_ptr<BlockInputMapItem>> values;
    for(const auto& object : objects.items())
    {
      nlohmann::json itemId = object.value().value("item_id", nlohmann::json());
      if(itemId.is_number_unsigned())
        values.emplace_back(std::make_shared<BlockInputMapItem>(*this, itemId.get<uint32_t>()));
      else
        break;
    }
    items.load(std::move(values));
  }
  SubObject::load(loader, data);
}

void BlockInputMap::worldEvent(WorldState state, WorldEvent event)
{
  SubObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit) && !contains(state, WorldState::Run);

  Attributes::setEnabled(add, editable);
  Attributes::setEnabled(remove, editable);
  Attributes::setEnabled(moveUp, editable);
  Attributes::setEnabled(moveDown, editable);
}

uint32_t BlockInputMap::getItemId() const
{
  uint32_t itemId = 1;
  while(std::find_if(items.begin(), items.end(), [itemId](const auto& it){ return it->itemId() == itemId; }) != items.end())
    itemId++;
  return itemId;
}
