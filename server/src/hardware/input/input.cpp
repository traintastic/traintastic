/**
 * server/src/hardware/input/input.cpp
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

#include "input.hpp"
#include "../../world/world.hpp"
#include "list/inputlisttablemodel.hpp"
#include "../../core/attributes.hpp"

Input::Input(const std::weak_ptr<World> world, std::string_view _id) :
  IdObject(world, _id),
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  value{this, "value", TriState::Undefined, PropertyFlags::ReadOnly | PropertyFlags::StoreState}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, "object:name");
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);
  Attributes::addValues(value, TriStateValues);
  m_interfaceItems.add(value);
}

void Input::addToWorld()
{
  IdObject::addToWorld();

  if(auto world = m_world.lock())
    world->inputs->addObject(shared_ptr<Input>());
}

void Input::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  name.setAttributeEnabled(editable);
}

void Input::updateValue(TriState _value)
{
  // todo: delay in ms for 0->1 || 1->0
  value.setValueInternal(_value);
  valueChanged(value);
}
