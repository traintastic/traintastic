/**
 * server/src/hardware/commandstation/rocoz21.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_ROCOZ21_HPP
#define TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_ROCOZ21_HPP

#include "commandstation.hpp"
#include <boost/asio.hpp>
#include "../../core/objectproperty.hpp"
#include "../protocol/loconet/loconet.hpp"

struct z21_lan_header;

namespace Z21 {
  class Message;
}

class RocoZ21 : public CommandStation
{
  protected:
    boost::asio::ip::udp::socket m_socket;
    boost::asio::ip::udp::endpoint m_remoteEndpoint;
    boost::asio::ip::udp::endpoint m_receiveEndpoint;
    std::array<uint8_t,64> m_receiveBuffer;

    bool setOnline(bool& value) final;
    //bool isDecoderSupported(Decoder& decoder) const final;
    void emergencyStopChanged(bool value) final;
    void powerOnChanged(bool value) final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    void receive();
    void send(const Z21::Message& message);
    void send(const z21_lan_header* msg);
    inline void send(const z21_lan_header& msg) { send(&msg); }

  public:
    CLASS_ID("command_station.z21")
    CREATE(RocoZ21)

    RocoZ21(const std::weak_ptr<World>& world, std::string_view _id);

    Property<std::string> hostname;
    Property<uint16_t> port;
    ObjectProperty<LocoNet::LocoNet> loconet;
    Property<std::string> serialNumber;
    Property<std::string> hardwareType;
    Property<std::string> firmwareVersion;
    Property<float> mainCurrent;
    Property<float> progCurrent;
    Property<float> filteredMainCurrent;
    Property<float> temperature;
    Property<float> supplyVoltage;
    Property<float> vccVoltage;
    //Property<bool> emergencyStop;
    //Property<bool> trackVoltageOff;
    Property<bool> shortCircuit;
    Property<bool> programmingModeActive;
    Property<bool> highTemperature;
    Property<bool> powerLost;
    Property<bool> shortCircutInternal;
    Property<bool> shortCircutExternal;
};

#endif
