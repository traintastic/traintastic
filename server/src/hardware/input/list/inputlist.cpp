/**
 * server/src/hardware/input/list/inputlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "inputlist.hpp"
#include "inputlisttablemodel.hpp"
#include "../inputs.hpp"
#include "../../../world/getworld.hpp"
#include "../../../core/attributes.hpp"

InputList::InputList(Object& _parent, const std::string& parentPropertyName) :
  ObjectList<Input>(_parent, parentPropertyName),
  add{*this, "add",
    [this](std::string_view classId)
    {
      auto world = getWorld(&this->parent());
      if(!world)
        return std::shared_ptr<Input>();
      auto input = Inputs::create(world, classId, world->getUniqueId("input"));
      if(auto locoNetInput = std::dynamic_pointer_cast<LocoNetInput>(input); locoNetInput && world->loconets->length == 1)
      {
        auto& loconet = world->loconets->operator[](0);
        if(uint16_t address = loconet->getUnusedInputAddress(); address != LocoNetInput::addressInvalid)
        {
          locoNetInput->address = address;
          locoNetInput->loconet = loconet;
        }
      }
      else if(auto xpressNetInput = std::dynamic_pointer_cast<XpressNetInput>(input); xpressNetInput && world->xpressnets->length == 1)
      {
        auto& xpressnet = world->xpressnets->operator[](0);
        if(uint16_t address = xpressnet->getUnusedInputAddress(); address != XpressNetInput::addressInvalid)
        {
          xpressNetInput->address = address;
          xpressNetInput->xpressnet = xpressnet;
        }
      }
      return input;
    }}
  , remove{*this, "remove",
    [this](const std::shared_ptr<Input>& input)
    {
      if(containsObject(input))
        input->destroy();
      assert(!containsObject(input));
    }}
{
  auto w = getWorld(&_parent);
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(add, editable);
  Attributes::addClassList(add, Inputs::classList);
  m_interfaceItems.add(add);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);
}

TableModelPtr InputList::getModel()
{
  return std::make_shared<InputListTableModel>(*this);
}

void InputList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Input>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(add, editable);
  Attributes::setEnabled(remove, editable);
}

bool InputList::isListedProperty(const std::string& name)
{
  return InputListTableModel::isListedProperty(name);
}
