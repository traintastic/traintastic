/**
 * server/src/hardware/output/output.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

Output::Output(World& world, std::string_view _id)
  : IdObject(world, _id)
  , name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , interface{this, "interface", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](const std::shared_ptr<OutputController>& /*newValue*/)
      {
        interfaceChanged();
      },
      [this](const std::shared_ptr<OutputController>& newValue)
      {
        if(interface.value() && !interface->removeOutput(*this))
          return false;

        if(newValue)
        {
          if(!newValue->isOutputChannel(channel))
          {
            const auto* const channels = newValue->outputChannels();
            if(channels && !channels->empty())
              channel.setValueInternal(channels->front());
            else
              channel.setValueInternal(OutputController::defaultOutputChannel);
          }

          if(!newValue->isOutputAddressAvailable(channel, address))
          {
            const uint32_t newAddress = newValue->getUnusedOutputAddress(channel);
            if(newAddress == Output::invalidAddress)
              return false; // no free address available
            address.setValueInternal(newAddress);
          }

          return newValue->addOutput(*this);
        }

        return true;
      }}
  , channel{this, "channel", OutputController::defaultOutputChannel, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](const uint32_t& /*newValue*/)
      {
        channelChanged();
      },
      [this](const uint32_t& newValue)
      {
        if(interface)
          return interface->changeOutputChannelAddress(*this, newValue, address);
        return true;
      }}
  , address{this, "address", 1, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
      [this](const uint32_t& newValue)
      {
        if(interface)
          return interface->changeOutputChannelAddress(*this, channel, newValue);
        return true;
      }}
  , value{this, "value", TriState::Undefined, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
  , controllers{*this, "controllers", {}, PropertyFlags::ReadWrite | PropertyFlags::NoStore}
  , setValue{*this, "set_value", MethodFlags::ScriptCallable,
      [this](bool newValue)
      {
        return interface && interface->setOutputValue(channel, address, newValue);
      }}
  , onValueChanged{*this, "on_value_changed", EventFlags::Scriptable}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, editable);
  Attributes::addObjectList(interface, m_world.outputControllers);
  m_interfaceItems.add(interface);

  Attributes::addDisplayName(channel, DisplayName::Hardware::channel);
  Attributes::addEnabled(channel, editable);
  Attributes::addVisible(channel, false);
  Attributes::addValues(channel, OutputController::noOutputChannels);
  Attributes::addAliases(channel, OutputController::noOutputChannels, nullptr);
  m_interfaceItems.add(channel);

  Attributes::addDisplayName(address, DisplayName::Hardware::address);
  Attributes::addEnabled(address, editable);
  Attributes::addVisible(address, false);
  Attributes::addMinMax(address, addressMinDefault, addressMaxDefault);
  m_interfaceItems.add(address);

  Attributes::addObjectEditor(value, false);
  Attributes::addValues(value, TriStateValues);
  m_interfaceItems.add(value);

  Attributes::addObjectEditor(controllers, false); //! \todo add client support first
  m_interfaceItems.add(controllers);

  Attributes::addObjectEditor(setValue, false);
  m_interfaceItems.add(setValue);

  m_interfaceItems.add(onValueChanged);
}

void Output::addToWorld()
{
  IdObject::addToWorld();
  m_world.outputs->addObject(shared_ptr<Output>());
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
    interfaceChanged();
  }
}

void Output::destroying()
{
  if(interface)
    interface = nullptr;
  m_world.outputs->removeObject(shared_ptr<Output>());
  IdObject::destroying();
}

void Output::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  Attributes::setEnabled({name, interface, channel, address}, contains(state, WorldState::Edit));
}

void Output::updateValue(TriState _value)
{
  value.setValueInternal(_value);
  if(value != TriState::Undefined)
    fireEvent(onValueChanged, value == TriState::True);
}

void Output::interfaceChanged()
{
  Attributes::setValues(channel, interface ? interface->outputChannels() : OutputController::noOutputChannels);
  Attributes::setAliases(channel, interface ? interface->outputChannels() : OutputController::noOutputChannels, interface ? interface->outputChannelNames() : nullptr);
  Attributes::setVisible(channel, interface && interface->outputChannels() && !interface->outputChannels()->empty());
  Attributes::setVisible(address, interface);

  channelChanged();
}

void Output::channelChanged()
{
  if(interface)
  {
    const auto limits = interface->outputAddressMinMax(channel);
    Attributes::setMinMax(address, limits.first, limits.second);
  }
  else
    Attributes::setMinMax(address, addressMinDefault, addressMaxDefault);

  value.setValueInternal(TriState::Undefined);
}
