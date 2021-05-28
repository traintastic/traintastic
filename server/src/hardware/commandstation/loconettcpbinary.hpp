/**
 * server/src/hardware/commandstation/loconettcpbinary.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_LOCONETTCPBINARY_HPP
#define TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_LOCONETTCPBINARY_HPP

#include "commandstation.hpp"
#include <boost/asio.hpp>
#include "../protocol/loconet/loconet.hpp"

class LocoNetTCPBinary : public CommandStation
{
  protected:
    boost::asio::ip::tcp::socket m_socket;
    std::array<uint8_t, 1024> m_readBuffer;
    uint16_t m_readBufferOffset;

    bool setOnline(bool& value) final;
    void emergencyStopChanged(bool value) final;
    void powerOnChanged(bool value) final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    void receive();
    bool send(const LocoNet::Message& msg);

  public:
    CLASS_ID("command_station.loconet_tcp_binary")
    CREATE(LocoNetTCPBinary)

    Property<std::string> hostname;
    Property<uint16_t> port;
    ObjectProperty<LocoNet::LocoNet> loconet;

    LocoNetTCPBinary(const std::weak_ptr<World>& world, std::string_view _id);
};

#endif
