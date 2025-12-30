/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#include "boosterlist.hpp"
#include "boosterlisttablemodel.hpp"
#include "../booster.hpp"
#include "../../../world/world.hpp"
#include "../../../world/getworld.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "../../../core/objectproperty.tpp"
#include "../../../utils/displayname.hpp"

BoosterList::BoosterList(Object& _parent, std::string_view parentPropertyName) :
  ObjectList<Booster>(_parent, parentPropertyName),
  create{*this, "create",
    [this]()
    {
      auto& world = getWorld(parent());
      return Booster::create(world, world.getUniqueId(Booster::defaultId));
    }},
  delete_{*this, "delete", std::bind(&BoosterList::deleteMethodHandler, this, std::placeholders::_1)}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(create, DisplayName::List::create);
  Attributes::addEnabled(create, editable);
  m_interfaceItems.add(create);

  Attributes::addDisplayName(delete_, DisplayName::List::delete_);
  Attributes::addEnabled(delete_, editable);
  m_interfaceItems.add(delete_);
}

TableModelPtr BoosterList::getModel()
{
  return std::make_shared<BoosterListTableModel>(*this);
}

void BoosterList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Booster>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(create, editable);
  Attributes::setEnabled(delete_, editable);
}

bool BoosterList::isListedProperty(std::string_view name)
{
  return BoosterListTableModel::isListedProperty(name);
}
