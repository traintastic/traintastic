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
#include "../../enum/decoderprotocol.hpp"
#include "../../enum/direction.hpp"

namespace Hardware {

class DecoderFunction;

class Decoder : public IdObject
{
  public:
    enum class ChangeFlags
    {
      EmergencyStop = 1 << 0,
      Direction = 1 << 1,
      SpeedStep = 1 << 2,
      SpeedSteps = 1 << 3,
      FunctionValue = 1 << 4,
      FunctionMomentary = 1 << 5,
    };

  public:
    CLASS_ID("hardware.decoder")

    //ObjectProperty<CommandStation::CommandStation> commandStation;
    Property<DecoderProtocol> protocol;
    Property<uint16_t> address;
    Property<bool> emergencyStop;
    Property<Direction> direction;
    Property<uint8_t> speedSteps;
    Property<uint8_t> speedStep;

    Decoder(const std::string& _id);

    const std::shared_ptr<DecoderFunction>& getFunction(uint32_t number) const;
};

constexpr Decoder::ChangeFlags operator| (const Decoder::ChangeFlags& lhs, const Decoder::ChangeFlags& rhs)
{
  return static_cast<Decoder::ChangeFlags>(static_cast<std::underlying_type_t<Decoder::ChangeFlags>>(lhs) | static_cast<std::underlying_type_t<Decoder::ChangeFlags>>(rhs));
}

constexpr bool has(const Decoder::ChangeFlags& value, const Decoder::ChangeFlags& mask)
{
  return (static_cast<std::underlying_type_t<Decoder::ChangeFlags>>(value) & static_cast<std::underlying_type_t<Decoder::ChangeFlags>>(mask)) != 0;
}

}

#endif
