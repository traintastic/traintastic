/**
 * server/src/hardware/protocol/xpressnet/xpressnet.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_XPRESSNET_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_XPRESSNET_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/property.hpp"
#include "../../../enum/xpressnetcommandstation.hpp"
#include "../../../hardware/decoder/decoderchangeflags.hpp"
#include "messages.hpp"

class CommandStation;
class Decoder;

namespace XpressNet {

class XpressNet : public SubObject
{
  protected:
    static bool getFunctionValue(const Decoder& decoder, uint32_t number);

    CommandStation* const m_commandStation; // valid if parent is command station, else nullptr
    std::function<bool(const Message&)> m_send;
    std::atomic_bool m_debugLog;

    void worldEvent(WorldState state, WorldEvent event) final;

  public:
    CLASS_ID("protocol.xpressnet")

    Property<XpressNetCommandStation> commandStation;
    Property<bool> useEmergencyStopLocomotiveCommand;
    Property<bool> useFunctionStateCommands;
    Property<bool> useRocoF13F20Command;
    Property<bool> debugLog;

    XpressNet(Object& _parent, const std::string& parentPropertyName, std::function<bool(const Message&)> send);

    bool send(const Message& msg) { return m_send(msg); }
    void receive(const Message& msg);

    void emergencyStopChanged(bool value);
    void trackVoltageOffChanged(bool value);
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);
};

}

#endif
