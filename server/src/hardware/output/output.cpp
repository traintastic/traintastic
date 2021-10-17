/**
 * server/src/hardware/output/output.cpp
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

#include "output.hpp"
#include "../../world/world.hpp"
#include "list/outputlisttablemodel.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"
#include "../../utils/displayname.hpp"

Output::Output(const std::weak_ptr<World> world, std::string_view _id)
  : IdObject(world, _id)
  , name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , interface{this, "interface", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](const std::shared_ptr<OutputController>& newValue)
      {
        if(newValue)
        {
          const auto limits = newValue->outputAddressMinMax();
          Attributes::setMinMax(address, limits.first, limits.second);
        }
        else
        {
          Attributes::setMinMax(address, addressMinDefault, addressMaxDefault);
          value.setValueInternal(TriState::Undefined);
        }
      },
      [this](const std::shared_ptr<OutputController>& newValue)
      {
        if(!newValue || newValue->addOutput(*this))
        {
          if(interface.value())
            interface->removeOutput(*this);
          return true;
        }
        return false;
      }}
  , address{this, "address", 1, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
      [this](const uint32_t& newValue)
      {
        if(interface)
          return interface->changeOutputAddress(*this, newValue);
        return true;
      }}
  , value{this, "value", TriState::Undefined, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
      [this](TriState newValue)
      {
        valueChanged(newValue);
      },
      [this](TriState& newValue) -> bool
      {
        if(!interface || newValue == TriState::Undefined)
          return false;
        else
          return interface->setOutputValue(address, newValue == TriState::True);
      }}
  , controllers{*this, "controllers", {}, PropertyFlags::ReadWrite | PropertyFlags::NoStore}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, editable);
  Attributes::addObjectList(interface, w->outputControllers);
  m_interfaceItems.add(interface);

  Attributes::addDisplayName(address, DisplayName::Hardware::address);
  Attributes::addEnabled(address, editable);
  Attributes::addMinMax(address, addressMinDefault, addressMaxDefault);
  m_interfaceItems.add(address);

  Attributes::addObjectEditor(value, false);
  Attributes::addValues(value, TriStateValues);
  m_interfaceItems.add(value);

  Attributes::addObjectEditor(controllers, false); //! \todo add client support first
  m_interfaceItems.add(controllers);
}

void Output::addToWorld()
{
  IdObject::addToWorld();

  if(auto world = m_world.lock())
    world->outputs->addObject(shared_ptr<Output>());
}

void Output::loaded()
{
  IdObject::loaded();
  if(interface)
  {
    if(!interface->addOutput(*this))
    {
      if(auto object = std::dynamic_pointer_cast<Object>(interface.value()))
        Log::log(*this, LogMessage::C2001_ADDRESS_ALREADY_USED_AT_X, *object);
      interface.setValueInternal(nullptr);
    }
  }
}

void Output::destroying()
{
  if(auto world = m_world.lock())
    world->outputs->removeObject(shared_ptr<Output>());
  if(interface)
    interface = nullptr;
  IdObject::destroying();
}

void Output::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(interface, editable);
  Attributes::setEnabled(address, editable);
}

void Output::updateValue(TriState _value)
{
  value.setValueInternal(_value);
  valueChanged(value);
}
