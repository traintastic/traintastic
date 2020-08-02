/**
 * server/src/hardware/commandstation/xpressnet/xpressnet.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_XPRESSNET_XPRESSNET_HPP
#define TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_XPRESSNET_XPRESSNET_HPP

#include "../commandstation.hpp"
#include "../../protocol/xpressnet.hpp"
#include "../../../enum/xpressnetcommandstation.hpp"

namespace CommandStation {

class XpressNet : public CommandStation
{
  protected:
    static bool getFunctionValue(const Decoder& decoder, uint32_t number);

    void worldEvent(WorldState state, WorldEvent event) override;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) override;

    virtual void send(const Protocol::XpressNet::Message& msg) = 0;

  public:

    Property<XpressNetCommandStation> commandStation;
    Property<bool> useEmergencyStopLocomotiveCommand;
    Property<bool> useFunctionStateCommands;
    Property<bool> useRocoF13F20Command;

    XpressNet(const std::weak_ptr<World>& world, std::string_view _id);
};

}

#endif
