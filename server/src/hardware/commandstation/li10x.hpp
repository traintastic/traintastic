/**
 * server/src/hardware/commandstation/li10x.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_LI10X_HPP
#define TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_LI10X_HPP

#include "commandstation.hpp"
#include "../protocol/xpressnet.hpp"
#include <boost/asio/serial_port.hpp>
//#include "../../core/objectproperty.hpp"
//#include "protocol/xpressnet.hpp"

namespace Hardware::CommandStation {

class LI10x : public CommandStation
{
  protected:
    boost::asio::serial_port m_serialPort;
    std::array<uint8_t, 32> m_readBuffer;
    std::unique_ptr<uint8_t[]> m_readMessage;
    uint8_t m_readMessageTodo;
    uint8_t m_readMessagePos;

    bool setOnline(bool& value) final;
    void emergencyStopChanged(bool value) final;
    void trackVoltageOffChanged(bool value) final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    bool start();
    void stop();
    bool send(const Protocol::XpressNet::Message& msg);
    void receive(std::unique_ptr<uint8_t[]> message);
    void read();

  public:
    CLASS_ID("hardware.command_station.li10x")
    CREATE(LI10x)

    Property<std::string> port;
    Property<uint32_t> baudrate;
    //Property<bool> useCTS;
    ObjectProperty<::Protocol::XpressNet> xpressnet;

    LI10x(const std::weak_ptr<World>& world, const std::string& _id);
};

}

#endif
