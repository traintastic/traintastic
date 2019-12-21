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
  name{this, "name", "", PropertyFlags::AccessWCC},
  online{this, "online", false, PropertyFlags::TODO, nullptr, std::bind(&CommandStation::setOnline, this, std::placeholders::_1)},
  status{this, "status", CommandStationStatus::Offline, PropertyFlags::AccessRRR},
  decoders{this, "decoders", std::make_shared<DecoderList>(world, world.lock()->getUniqueId("decoders")), PropertyFlags::TODO},
  notes{this, "notes", "", PropertyFlags::AccessWWW}
{
  m_interfaceItems.add(name);
  m_interfaceItems.add(online);
  m_interfaceItems.add(status);
  m_interfaceItems.add(decoders);
  m_interfaceItems.add(notes)
    .addAttributeCategory(Category::Notes);
}

const std::shared_ptr<Decoder>& CommandStation::getDecoder(DecoderProtocol protocol, uint16_t address, bool longAddress) const
{


  return Decoder::null;
}

}
