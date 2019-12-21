/**
 * Traintastic
 *
 * Copyright (C) 2019 Reinder Feenstra <reinderfeenstra@gmail.com>
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
#include "../commandstation/commandstation.hpp"

namespace Hardware {

const std::shared_ptr<Decoder> Decoder::null;

Decoder::Decoder(const std::weak_ptr<World>& world, const std::string& _id) :
  IdObject(world, _id),
  name{this, "name", "", PropertyFlags::AccessWCC},
  commandStation{this, "command_station", nullptr, PropertyFlags::AccessWCC,
    [this](const std::shared_ptr<CommandStation::CommandStation>& value)
    {
      std::shared_ptr<Decoder> decoder = std::dynamic_pointer_cast<Decoder>(shared_from_this());
      assert(decoder);

      //if(value)
      // TODO: check compatible??

      if(commandStation)
        commandStation->decoders->remove(decoder);

      if(value)
        value->decoders->add(decoder);

      return true;
    }},
  protocol{this, "protocol", DecoderProtocol::None, PropertyFlags::AccessWCC},
  address{this, "address", 0, PropertyFlags::AccessWCC},
  longAddress{this, "long_address", false, PropertyFlags::AccessWCC},
  emergencyStop{this, "emergency_stop", false, PropertyFlags::AccessRWW,
    [this](const bool&)
    {
      changed(DecoderChangeFlags::EmergencyStop);
    }},
  direction{this, "direction", Direction::Forward, PropertyFlags::AccessWWW,
    [this](const Direction&)
    {
      changed(DecoderChangeFlags::Direction);
    }},
  speedSteps{this, "speed_steps", 255, PropertyFlags::AccessWCC,
    [this](const uint8_t&)
    {
      changed(DecoderChangeFlags::SpeedSteps);
    }},
  speedStep{this, "speed_step", 0, PropertyFlags::AccessRRW,
    [this](const uint8_t&)
    {
      changed(DecoderChangeFlags::SpeedStep);
    },
    [this](uint8_t& value)
    {
      return (value <= speedSteps);
    }},
  functions{this, "functions", std::make_shared<DecoderFunctionList>(world, _id + "_functions"), PropertyFlags::AccessRRR},
  notes{this, "notes", "", PropertyFlags::AccessWWW}
{
  m_interfaceItems.add(name);
  m_interfaceItems.add(commandStation)
    .addAttributeEnabled(false);
  m_interfaceItems.add(protocol)
    .addAttributeEnabled(false);
  m_interfaceItems.add(address)
    .addAttributeEnabled(false);
  m_interfaceItems.add(emergencyStop)
    .addAttributeEnabled(false);
  m_interfaceItems.add(direction)
    .addAttributeEnabled(false);
  m_interfaceItems.add(speedSteps)
    .addAttributeEnabled(false);
  m_interfaceItems.add(speedStep)
    .addAttributeEnabled(false);
  m_interfaceItems.add(functions);
  m_interfaceItems.add(notes);
}

const std::shared_ptr<DecoderFunction>& Decoder::getFunction(uint32_t number) const
{
  for(auto& f : *functions)
    if(f->number == number)
      return f;

  return DecoderFunction::null;
}

void Decoder::modeChanged(TraintasticMode mode)
{
  IdObject::modeChanged(mode);

  commandStation.setAttributeEnabled(mode == TraintasticMode::Edit);
  protocol.setAttributeEnabled(mode == TraintasticMode::Edit);
  address.setAttributeEnabled(mode == TraintasticMode::Edit);
  speedSteps.setAttributeEnabled(mode == TraintasticMode::Edit);
  speedStep.setAttributeEnabled(mode == TraintasticMode::Run);

  if(mode == TraintasticMode::Edit)
    speedStep = 0;
}

void Decoder::changed(DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(commandStation)
    commandStation->decoderChanged(*this, changes, functionNumber);
}

}
