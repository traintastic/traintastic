/**
 * server/src/hardware/decoder/decoder.hpp
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

#ifndef SERVER_HARDWARE_DECODER_DECODER_HPP
#define SERVER_HARDWARE_DECODER_DECODER_HPP

#include <type_traits>
#include "../../core/idobject.hpp"
#include "../../core/objectproperty.hpp"
#include "../../enum/decoderprotocol.hpp"
#include "../../enum/direction.hpp"
#include "decoderfunctionlist.hpp"

class DecoderFunctionList;

namespace Hardware {

enum class DecoderChangeFlags;
class DecoderFunction;

namespace CommandStation {
  class CommandStation;
}

class Decoder : public IdObject
{
  friend class DecoderFunction;

  protected:
    void modeChanged(TraintasticMode mode) final;
    void changed(DecoderChangeFlags changes, uint32_t functionNumber = 0);

  public:
    CLASS_ID("hardware.decoder")
    CREATE(Decoder)

    static const std::shared_ptr<Decoder> null;

    Decoder(const std::weak_ptr<World>& world, const std::string& _id);

    Property<std::string> name;
    ObjectProperty<CommandStation::CommandStation> commandStation;
    Property<DecoderProtocol> protocol;
    Property<uint16_t> address;
    Property<bool> longAddress;
    Property<bool> emergencyStop;
    Property<Direction> direction;
    Property<uint8_t> speedSteps;
    Property<uint8_t> speedStep;
    ObjectProperty<DecoderFunctionList> functions;
    Property<std::string> notes;

    const std::shared_ptr<DecoderFunction>& getFunction(uint32_t number) const;
};

}

#endif
