/**
 * server/src/hardware/input/input.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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
#include "inputlisttablemodel.hpp"

Input::Input(const std::weak_ptr<World> world, std::string_view _id) :
  IdObject(world, _id),
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  value{this, "value", false, PropertyFlags::ReadOnly | PropertyFlags::StoreState}
{
  m_interfaceItems.add(name);
  m_interfaceItems.add(value);
}

void Input::addToWorld()
{
  IdObject::addToWorld();

  if(auto world = m_world.lock())
    world->inputs->addObject(shared_ptr<Input>());
}

void Input::valueChanged(bool _value)
{
  // todo: delay in ms for 0->1 || 1->0
  value.setValueInternal(_value);
}
