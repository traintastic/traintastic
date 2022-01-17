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
#include "../../log/log.hpp"
#include "../../utils/displayname.hpp"

Input::Input(World& world, std::string_view _id)
  : IdObject(world, _id)
  , name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , interface{this, "interface", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
      [this](const std::shared_ptr<InputController>& newValue)
      {
        if(!newValue || newValue->addInput(*this))
        {
          if(interface.value())
            return interface->removeInput(*this);
          return true;
        }
        return false;
      }}
  , address{this, "address", 1, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
      [this](const uint32_t& newValue)
      {
        if(interface)
          return interface->changeInputAddress(*this, newValue);
        return true;
      }}
  , value{this, "value", TriState::Undefined, PropertyFlags::ReadOnly | PropertyFlags::StoreState}
  , consumers{*this, "consumers", {}, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, editable);
  Attributes::addObjectList(interface, m_world.inputControllers);
  m_interfaceItems.add(interface);

  Attributes::addDisplayName(address, DisplayName::Hardware::address);
  Attributes::addEnabled(address, editable);
  Attributes::addMinMax(address, std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
  m_interfaceItems.add(address);

  Attributes::addObjectEditor(value, false);
  Attributes::addValues(value, TriStateValues);
  m_interfaceItems.add(value);

  Attributes::addObjectEditor(consumers, false); //! \todo add client support first
  m_interfaceItems.add(consumers);
}

void Input::addToWorld()
{
  IdObject::addToWorld();

  m_world.inputs->addObject(shared_ptr<Input>());
}

void Input::loaded()
{
  IdObject::loaded();
  if(interface)
  {
    if(!interface->addInput(*this))
    {
      if(auto object = std::dynamic_pointer_cast<Object>(interface.value()))
        Log::log(*this, LogMessage::C2001_ADDRESS_ALREADY_USED_AT_X, *object);
      interface.setValueInternal(nullptr);
    }
  }
}

void Input::destroying()
{
  if(interface.value())
    interface = nullptr;
  m_world.inputs->removeObject(shared_ptr<Input>());
  IdObject::destroying();
}

void Input::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(interface, editable);
  Attributes::setEnabled(address, editable);
}

void Input::updateValue(TriState _value)
{
  // todo: delay in ms for 0->1 || 1->0
  value.setValueInternal(_value);
  valueChanged(value);
}
