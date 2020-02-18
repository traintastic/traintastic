/**
 * server/src/hardware/commandstation/commandstation.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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
#include <functional>
#include "../../core/world.hpp"
#include "../decoder/decoder.hpp"
#include "../decoder/decoderlist.hpp"

namespace Hardware::CommandStation {

CommandStation::CommandStation(const std::weak_ptr<World>& world, const std::string& _id) :
  IdObject(world, _id),
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  online{this, "online", false, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](bool value)
    {
      emergencyStop.setAttributeEnabled(value);
      trackVoltageOff.setAttributeEnabled(value);
    },
    std::bind(&CommandStation::setOnline, this, std::placeholders::_1)},
  //status{this, "status", CommandStationStatus::Offline, PropertyFlags::ReadOnly},
  emergencyStop{this, "emergency_stop", false, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](bool value)
    {
      emergencyStopChanged(value);
    }},
  trackVoltageOff{this, "track_voltage_off", false, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](bool value)
    {
      trackVoltageOffChanged(value);
    }},
  decoders{this, "decoders", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  notes{this, "notes", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  decoders.setValueInternal(std::make_shared<DecoderList>(*this, decoders.name()));

  m_interfaceItems.add(name)
    .addAttributeEnabled(false);
  m_interfaceItems.add(online);
  m_interfaceItems.insertBefore(emergencyStop, notes)
    .addAttributeEnabled(false)
    .addAttributeObjectEditor(false);
  m_interfaceItems.insertBefore(trackVoltageOff, notes)
    .addAttributeEnabled(false)
    .addAttributeObjectEditor(false);
  m_interfaceItems.add(decoders);
  m_interfaceItems.add(notes);
}

void CommandStation::modeChanged(TraintasticMode mode)
{
  IdObject::modeChanged(mode);

  name.setAttributeEnabled(mode == TraintasticMode::Edit);
}

const std::shared_ptr<Decoder>& CommandStation::getDecoder(DecoderProtocol protocol, uint16_t address, bool longAddress) const
{
  auto it = std::find_if(decoders->begin(), decoders->end(), [=](auto& decoder){ return decoder->protocol == protocol && decoder->address == address && decoder->longAddress == longAddress; });
  if(it != decoders->end())
    return *it;
  return Decoder::null;
}

}
