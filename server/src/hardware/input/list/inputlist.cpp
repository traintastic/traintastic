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
#include "../inputcontroller.hpp"
#include "../../../world/getworld.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"

InputList::InputList(Object& _parent, const std::string& parentPropertyName)
  : ObjectList<Input>(_parent, parentPropertyName)
  , m_parentIsInputController(dynamic_cast<InputController*>(&_parent))
  , add{*this, "add",
      [this]()
      {
        auto world = getWorld(&parent());
        if(!world)
          return std::shared_ptr<Input>();

        auto input = Input::create(world, world->getUniqueId(Input::defaultId));
        if(const auto controller = std::dynamic_pointer_cast<InputController>(parent().shared_from_this()))
        {
          if(const uint32_t address = controller->getUnusedInputAddress(); address != Input::invalidAddress)
          {
            input->address = address;
            input->interface = controller;
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
  , inputMonitor{*this, "input_monitor",
      [this]()
      {
        if(const auto controller = std::dynamic_pointer_cast<InputController>(parent().shared_from_this()))
          return controller->inputMonitor();
        return std::shared_ptr<InputMonitor>();
      }}
{
  auto w = getWorld(&_parent);
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, editable);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);

  if(m_parentIsInputController)
  {
    Attributes::addDisplayName(inputMonitor, DisplayName::Hardware::inputMonitor);
    m_interfaceItems.add(inputMonitor);
  }
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
