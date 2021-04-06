/**
 * server/src/hardware/protocol/xpressnet/xpressnet.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_XPRESSNET_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_XPRESSNET_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/property.hpp"
#include "../../../core/method.hpp"
#include "../../../enum/xpressnetcommandstation.hpp"
#include "../../../hardware/decoder/decoderchangeflags.hpp"
#include "messages.hpp"
#include "xpressnetinputmonitor.hpp"

class CommandStation;
class Decoder;
class XpressNetInput;
class XpressNetInputMonitor;

namespace XpressNet {

class XpressNet : public SubObject
{
  friend class ::XpressNetInputMonitor;

  protected:
    static bool getFunctionValue(const Decoder& decoder, uint32_t number);

    CommandStation* const m_commandStation; // valid if parent is command station, else nullptr
    std::function<bool(const Message&)> m_send;
    std::atomic_bool m_debugLog;
    std::unordered_map<uint16_t, std::shared_ptr<XpressNetInput>> m_inputs;
    std::vector<XpressNetInputMonitor*> m_inputMonitors;

    void worldEvent(WorldState state, WorldEvent event) final;

  public:
    CLASS_ID("protocol.xpressnet")

    Property<XpressNetCommandStation> commandStation;
    Property<bool> useEmergencyStopLocomotiveCommand;
    //Property<bool> useFunctionStateCommands;
    Property<bool> useRocoF13F20Command;
    Property<bool> debugLog;
    Method<std::shared_ptr<XpressNetInputMonitor>()> inputMonitor;

    XpressNet(Object& _parent, const std::string& parentPropertyName, std::function<bool(const Message&)> send);

    bool send(const Message& msg) { return m_send(msg); }
    void receive(const Message& msg);

    void emergencyStopChanged(bool value);
    void powerOnChanged(bool value);
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);

    [[nodiscard]] bool isInputAddressAvailable(uint16_t address) const;
    [[nodiscard]] bool changeInputAddress(XpressNetInput& input, uint16_t newAddress);
    [[nodiscard]] bool addInput(XpressNetInput& input);
    void removeInput(XpressNetInput& input);
    void inputMonitorIdChanged(uint32_t address, std::string_view value);
    void inputMonitorValueChanged(uint32_t address, TriState value);
};

}

#endif
