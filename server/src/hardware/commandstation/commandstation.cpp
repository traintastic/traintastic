/**
 * server/src/hardware/commandstation/commandstation.cpp
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

#include "commandstation.hpp"
#include "commandstationlist.hpp"
#include "commandstationlisttablemodel.hpp"
#include <functional>
#include "../../world/world.hpp"
#include "../decoder/decoder.hpp"
#include "../decoder/decoderlist.hpp"
#include "../../core/attributes.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/almostzero.hpp"

CommandStation::CommandStation(const std::weak_ptr<World>& world, std::string_view _id) :
  IdObject(world, _id),
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  online{this, "online", false, PropertyFlags::ReadWrite | PropertyFlags::NoStore,
    [this](bool value)
    {
      emergencyStop.setAttributeEnabled(value);
      powerOn.setAttributeEnabled(value);
    },
    std::bind(&CommandStation::setOnline, this, std::placeholders::_1)},
  //status{this, "status", CommandStationStatus::Offline, PropertyFlags::ReadOnly},
  emergencyStop{this, "emergency_stop", true, PropertyFlags::ReadWrite | PropertyFlags::NoStore,
    [this](bool value)
    {
      emergencyStopChanged(value);
    }},
  powerOn{this, "power_on", false, PropertyFlags::ReadWrite | PropertyFlags::NoStore,
    [this](bool value)
    {
      powerOnChanged(value);
    }},
  decoders{this, "decoders", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  controllers{this, "controllers", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  notes{this, "notes", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  decoders.setValueInternal(std::make_shared<DecoderList>(*this, decoders.name()));
  controllers.setValueInternal(std::make_shared<ControllerList>(*this, controllers.name()));

  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  m_interfaceItems.add(online);

  Attributes::addEnabled(emergencyStop, online);
  Attributes::addObjectEditor(emergencyStop, false);
  m_interfaceItems.insertBefore(emergencyStop, notes);

  Attributes::addEnabled(powerOn, online);
  Attributes::addObjectEditor(powerOn, false);
  m_interfaceItems.insertBefore(powerOn, notes);

  m_interfaceItems.add(decoders);

  m_interfaceItems.add(controllers);

  Attributes::addDisplayName(notes, DisplayName::Object::notes);
  m_interfaceItems.add(notes);
}

void CommandStation::loaded()
{
  IdObject::loaded();

  checkAllDecoders();
}

void CommandStation::addToWorld()
{
  IdObject::addToWorld();

  if(auto world = m_world.lock())
    world->commandStations->addObject(shared_ptr<CommandStation>());
}

void CommandStation::destroying()
{
  for(const auto& decoder : *decoders)
  {
    assert(decoder->commandStation.value() == shared_ptr<CommandStation>());
    decoder->commandStation = nullptr;
  }
  if(auto world = m_world.lock())
    world->commandStations->removeObject(shared_ptr<CommandStation>());
  IdObject::destroying();
}

void CommandStation::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  name.setAttributeEnabled(contains(state, WorldState::Edit));

  try
  {
    switch(event)
    {
      case WorldEvent::Offline:
        online = false;
        break;

      case WorldEvent::Online:
        online = true;
        break;

      case WorldEvent::PowerOff:
        powerOn = false;
        break;

      case WorldEvent::PowerOn:
        powerOn = true;
        if(!emergencyStop)
          restoreSpeed();
        break;

      case WorldEvent::Stop:
        emergencyStop = true;
        break;

      case WorldEvent::Run:
        emergencyStop = false;
        if(powerOn)
          restoreSpeed();
        break;

      default:
        break;
    }
  }
  catch(...)
  {
  }
}

const std::shared_ptr<::Decoder>& CommandStation::getDecoder(DecoderProtocol protocol, uint16_t address, bool longAddress) const
{
  auto it = std::find_if(decoders->begin(), decoders->end(), [=](auto& decoder){ return decoder->protocol.value() == protocol && decoder->address.value() == address && decoder->longAddress == longAddress; });
  if(it != decoders->end())
    return *it;
  return Decoder::null;
}

void CommandStation::emergencyStopChanged(bool value)
{
  for(auto& controller : *controllers)
    controller->emergencyStopChanged(value);
}

void CommandStation::powerOnChanged(bool value)
{
  for(auto& controller : *controllers)
    controller->powerOnChanged(value);
}

void CommandStation::checkAllDecoders() const
{
  for(const auto& decoder : *decoders)
    checkDecoder(*decoder);
}

void CommandStation::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  for(auto& controller : *controllers)
    controller->decoderChanged(decoder, changes, functionNumber);
}

//! \brief restore speed of all decoders that are not (emergency) stopped
void CommandStation::restoreSpeed()
{
  for(const auto& decoder : *decoders)
    if(!decoder->emergencyStop && !almostZero(decoder->throttle.value()))
      decoderChanged(*decoder, DecoderChangeFlags::Throttle, 0);
}
