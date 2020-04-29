/**
 * Traintastic
 *
 * Copyright (C) 2019-2020 Reinder Feenstra <reinderfeenstra@gmail.com>
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

#include "decoder.hpp"
#include "decoderlist.hpp"
#include "decoderlisttablemodel.hpp"
#include "decoderchangeflags.hpp"
#include "decoderfunction.hpp"
#include "decoderfunctionlist.hpp"
#include "../../core/world.hpp"
#include "../commandstation/commandstation.hpp"

namespace Hardware {

const std::shared_ptr<Decoder> Decoder::null;

Decoder::Decoder(const std::weak_ptr<World>& world, std::string_view _id) :
  IdObject(world, _id),
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  commandStation{this, "command_station", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const std::shared_ptr<CommandStation::CommandStation>& value)
    {
      std::shared_ptr<Decoder> decoder = std::dynamic_pointer_cast<Decoder>(shared_from_this());
      assert(decoder);

      //if(value)
      // TODO: check compatible??

      if(commandStation)
        commandStation->decoders->removeObject(decoder);

      if(value)
        value->decoders->addObject(decoder);

      return true;
    }},
  protocol{this, "protocol", DecoderProtocol::None, PropertyFlags::ReadWrite | PropertyFlags::Store},
  address{this, "address", 0, PropertyFlags::ReadWrite | PropertyFlags::Store},
  longAddress{this, "long_address", false, PropertyFlags::ReadWrite | PropertyFlags::Store},
  emergencyStop{this, "emergency_stop", false, PropertyFlags::ReadWrite,
    [this](const bool&)
    {
      changed(DecoderChangeFlags::EmergencyStop);
    }},
  direction{this, "direction", Direction::Forward, PropertyFlags::ReadWrite,
    [this](const Direction&)
    {
      changed(DecoderChangeFlags::Direction);
    }},
  speedSteps{this, "speed_steps", 255, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const uint8_t&)
    {
      changed(DecoderChangeFlags::SpeedSteps);
    }},
  speedStep{this, "speed_step", 0, PropertyFlags::ReadWrite,
    [this](const uint8_t& value)
    {
      changed(DecoderChangeFlags::SpeedStep);

      if(value == 0)
      {
        auto w = m_world.lock();
        setEditable(w && contains(w->state.value(), WorldState::Edit));
      }
      else
        setEditable(false);
    },
    [this](uint8_t& value)
    {
      return (value <= speedSteps);
    }},
  functions{this, "functions", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  notes{this, "notes", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  functions.setValueInternal(std::make_shared<DecoderFunctionList>(*this, functions.name()));

  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit) && speedStep == 0;

  m_interfaceItems.add(name)
    .addAttributeEnabled(editable);
  m_interfaceItems.add(commandStation)
    .addAttributeEnabled(editable);
  m_interfaceItems.add(protocol)
    .addAttributeEnabled(editable);
  m_interfaceItems.add(address)
    .addAttributeEnabled(editable);
  m_interfaceItems.add(emergencyStop);
  m_interfaceItems.add(direction);
  m_interfaceItems.add(speedSteps)
    .addAttributeEnabled(editable);
  m_interfaceItems.add(speedStep);
  m_interfaceItems.add(functions);
  m_interfaceItems.add(notes);
}

void Decoder::addToWorld()
{
  IdObject::addToWorld();

  if(auto world = m_world.lock())
    world->decoders->addObject(shared_ptr<Decoder>());
}

const std::shared_ptr<DecoderFunction>& Decoder::getFunction(uint32_t number) const
{
  for(auto& f : *functions)
    if(f->number == number)
      return f;

  return DecoderFunction::null;
}

void Decoder::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);
  setEditable(contains(state, WorldState::Edit) && speedStep == 0);
}

void Decoder::setEditable(bool value)
{
  commandStation.setAttributeEnabled(value);
  commandStation.setAttributeEnabled(value);
  protocol.setAttributeEnabled(value);
  address.setAttributeEnabled(value);
  speedSteps.setAttributeEnabled(value);
}

void Decoder::changed(DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(commandStation)
    commandStation->decoderChanged(*this, changes, functionNumber);
}

}
