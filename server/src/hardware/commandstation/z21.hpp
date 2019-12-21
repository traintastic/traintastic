/**
 * server/src/hardware/commandstation/z21.hpp
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

#ifndef SERVER_HARDWARE_COMMANDSTATION_Z21_HPP
#define SERVER_HARDWARE_COMMANDSTATION_Z21_HPP

#include "commandstation.hpp"
#include <boost/asio.hpp>
#include "../../core/objectproperty.hpp"
#include "protocol/xpressnet.hpp"

struct z21_lan_header;

namespace Hardware::CommandStation {

class Z21 : public CommandStation
{
  protected:
    boost::asio::ip::udp::socket m_socket;
    boost::asio::ip::udp::endpoint m_remoteEndpoint;
    boost::asio::ip::udp::endpoint m_receiveEndpoint;
    std::array<uint8_t,64> m_receiveBuffer;

    bool setOnline(bool& value) final;
    bool isDecoderSupported(Decoder& decoder) const final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    void receive();
    void send(const z21_lan_header* msg);
    inline void send(const z21_lan_header& msg) { send(&msg); }

  public:
    CLASS_ID("hardware.command_station.z21")
    CREATE(Z21)

    Z21(const std::weak_ptr<World>& world, const std::string& _id);

    Property<std::string> hostname;
    Property<uint16_t> port;
    Property<std::string> serialNumber;
    Property<std::string> hardwareType;
    Property<std::string> firmwareVersion;
    Property<bool> emergencyStop;
    Property<bool> trackVoltageOff;
};

}

#endif
