/**
 * server/src/hardware/identification/identification.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "identification.hpp"
#include "list/identificationlist.hpp"
#include "../../world/world.hpp"
#include "list/identificationlisttablemodel.hpp"
#include "../interface/loconetinterface.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../utils/displayname.hpp"

Identification::Identification(World& world, std::string_view _id)
  : IdObject(world, _id)
  , name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , interface{this, "interface", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](const std::shared_ptr<IdentificationController>& /*newValue*/)
      {
        interfaceChanged();
      },
      [this](const std::shared_ptr<IdentificationController>& newValue)
      {
        if(interface.value() && !interface->removeIdentification(*this))
          return false;

        if(newValue)
        {
          if(!newValue->isIdentificationChannel(channel))
          {
            const auto* const channels = newValue->identificationChannels();
            if(channels && !channels->empty())
              channel.setValueInternal(channels->front());
            else
              channel.setValueInternal(IdentificationController::defaultIdentificationChannel);
          }

          if(!newValue->isIdentificationAddressAvailable(channel, address))
          {
            const uint32_t newAddress = newValue->getUnusedIdentificationAddress(channel);
            if(newAddress == Identification::invalidAddress)
              return false; // no free address available
            address.setValueInternal(newAddress);
          }

          return newValue->addIdentification(*this);
        }

        return true;
      }}
  , channel{this, "channel", IdentificationController::defaultIdentificationChannel, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](const uint32_t& /*newValue*/)
      {
        channelChanged();
      },
      [this](const uint32_t& newValue)
      {
        if(interface)
          return interface->changeIdentificationChannelAddress(*this, newValue, address);
        return true;
      }}
  , address{this, "address", 1, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
      [this](const uint32_t& newValue)
      {
        if(interface)
          return interface->changeIdentificationChannelAddress(*this, channel, newValue);
        return true;
      }}
  , opcMultiSenseDirection{this, "opc_multi_sense_direction", OPCMultiSenseDirection::None, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , consumers{*this, "consumers", {}, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , onEvent{*this, "on_event", EventFlags::Scriptable}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, editable);
  Attributes::addObjectList(interface, m_world.identificationControllers);
  m_interfaceItems.add(interface);

  Attributes::addDisplayName(channel, DisplayName::Hardware::channel);
  Attributes::addEnabled(channel, editable);
  Attributes::addVisible(channel, false);
  Attributes::addValues(channel, IdentificationController::noIdentificationChannels);
  Attributes::addAliases(channel, IdentificationController::noIdentificationChannels, nullptr);
  m_interfaceItems.add(channel);

  Attributes::addDisplayName(address, DisplayName::Hardware::address);
  Attributes::addEnabled(address, editable);
  Attributes::addVisible(address, false);
  Attributes::addMinMax(address, addressMinDefault, addressMaxDefault);
  m_interfaceItems.add(address);

  Attributes::addEnabled(opcMultiSenseDirection, editable);
  Attributes::addVisible(opcMultiSenseDirection, false);
  Attributes::addValues(opcMultiSenseDirection, opcMultiSenseDirectionValues);
  m_interfaceItems.add(opcMultiSenseDirection);

  Attributes::addObjectEditor(consumers, false); //! \todo add client support first
  m_interfaceItems.add(consumers);

  m_interfaceItems.add(onEvent);
}

void Identification::addToWorld()
{
  IdObject::addToWorld();

  m_world.identifications->addObject(shared_ptr<Identification>());
}

void Identification::loaded()
{
  IdObject::loaded();
  if(interface)
  {
    if(!interface->addIdentification(*this))
    {
      if(auto object = std::dynamic_pointer_cast<Object>(interface.value()))
        Log::log(*this, LogMessage::C2001_ADDRESS_ALREADY_USED_AT_X, *object);
      interface.setValueInternal(nullptr);
    }
    interfaceChanged();
  }
}

void Identification::destroying()
{
  if(interface.value())
    interface = nullptr;
  m_world.identifications->removeObject(shared_ptr<Identification>());
  IdObject::destroying();
}

void Identification::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(interface, editable);
  Attributes::setEnabled(channel, editable);
  Attributes::setEnabled(address, editable);
  Attributes::setEnabled(opcMultiSenseDirection, editable);
}

void Identification::fireEvent(IdentificationEventType type, uint16_t identifier, Direction direction, uint8_t category)
{
  Object::fireEvent(onEvent, type, identifier, direction, category);
}

void Identification::interfaceChanged()
{
  const bool opcMultiSenseVisible = hasLocoNetInterface();

  Attributes::setValues(channel, interface ? interface->identificationChannels() : IdentificationController::noIdentificationChannels);
  Attributes::setAliases(channel, interface ? interface->identificationChannels() : IdentificationController::noIdentificationChannels, interface ? interface->identificationChannelNames() : nullptr);
  Attributes::setVisible(channel, interface && interface->identificationChannels() && !interface->identificationChannels()->empty());
  Attributes::setVisible(address, interface);
  Attributes::setVisible(opcMultiSenseDirection, opcMultiSenseVisible);

  channelChanged();
}

void Identification::channelChanged()
{
  if(interface)
  {
    const auto limits = interface->identificationAddressMinMax(channel);
    Attributes::setMinMax(address, limits.first, limits.second);
  }
  else
    Attributes::setMinMax(address, addressMinDefault, addressMaxDefault);
}

bool Identification::hasLocoNetInterface() const
{
  return dynamic_cast<const LocoNetInterface*>(&*interface);
}
