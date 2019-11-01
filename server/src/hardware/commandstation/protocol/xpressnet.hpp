/**
 * server/src/hardware/commandstation/protocol/xpressnet.hpp
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

#ifndef SERVER_HARDWARE_COMMANDSTATION_PROTOCOL_XPRESSNET_HPP
#define SERVER_HARDWARE_COMMANDSTATION_PROTOCOL_XPRESSNET_HPP

#include "../../../core/object.hpp"
#include "../../../core/property.hpp"
#include "../../../enum/xpressnetcommandstation.hpp"
#include "../../decoder/decoder.hpp"

namespace Hardware::CommandStation::Protocol {

class XpressNet : public Object
{
  protected:
    static constexpr uint16_t addressMin = 1;
    static constexpr uint16_t addressMax = 9999;

    static uint8_t calcChecksum(const void* cmd);

    void sendEmergencyStop(const Decoder& decoder);
    void sendSpeedAndDirectionInstruction(const Decoder& decoder);
    void sendFunctionInstructionGroup1(const Decoder& decoder);
    void sendFunctionInstructionGroup2(const Decoder& decoder);
    void sendFunctionInstructionGroup3(const Decoder& decoder);
    void sendSetFunctionStateGroup1(const Decoder& decoder);
    void sendSetFunctionStateGroup2(const Decoder& decoder);
    void sendSetFunctionStateGroup3(const Decoder& decoder);

  public:
    CLASS_ID("hardware.command_station.protocol.xpressnet")

    Property<XpressNetCommandStation> commandStation;
    Property<bool> useFunctionStateCommands;

    XpressNet();

    bool isDecoderSupported(const Decoder& decoder) const;

    void decoderChanged(const Decoder& decoder, Decoder::ChangeFlags changes, uint32_t functionNumber);
};

}

#endif
