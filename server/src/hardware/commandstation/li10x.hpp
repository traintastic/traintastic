/**
 * server/src/hardware/commandstation/li10x.hpp
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

#ifndef SERVER_HARDWARE_COMMANDSTATION_LI10X_HPP
#define SERVER_HARDWARE_COMMANDSTATION_LI10X_HPP

#include "commandstation.hpp"
#include "protocol/xpressnet.hpp"

namespace Hardware::CommandStation {

class LI10x : public CommandStation
{
  protected:
    std::shared_ptr<Protocol::XpressNet> m_xpressnet;

    bool isDecoderSupported(Decoder& decoder) const final;

  public:
    CLASS_ID("hardware.command_station.li10x")

    Property<std::string> port;
    Property<uint32_t> baudrate;
    //Property<bool> useCTS;
    //ObjectProperty<XpressNet> xpressnet;

    LI10x(const std::string& _id);
};

}

#endif
