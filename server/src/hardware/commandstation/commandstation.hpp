/**
 * server/src/hardware/commandstation/commandstation.hpp
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

#ifndef SERVER_HARDWARE_COMMANDSTATION_COMMANDSTATION_HPP
#define SERVER_HARDWARE_COMMANDSTATION_COMMANDSTATION_HPP

#include "../../core/idobject.hpp"
#include "../../core/objectproperty.hpp"
#include "../../enum/commandstationstatus.hpp"
#include <enum/decoderprotocol.hpp>

class DecoderList;

namespace Hardware {
  class Decoder;
  enum class DecoderChangeFlags;
}

namespace Hardware::CommandStation {

class CommandStation : public IdObject
{
  friend class ::Hardware::Decoder;

  protected:
    virtual bool setOnline(bool& value) = 0;
    virtual bool isDecoderSupported(Decoder& decoder) const = 0;
    virtual void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) = 0;

    const std::shared_ptr<Decoder>& getDecoder(DecoderProtocol protocol, uint16_t address, bool longAddress) const;

  public:
    CommandStation(const std::weak_ptr<World>& world, const std::string& _id);

    Property<std::string> name;
    Property<bool> online;
    Property<CommandStationStatus> status;
    ObjectProperty<DecoderList> decoders;
    Property<std::string> notes;
};

}

#endif

