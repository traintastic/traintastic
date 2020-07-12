/**
 * server/src/hardware/commandstation/loconetserial.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_LOCONETSERIAL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_LOCONETSERIAL_HPP

#include "commandstation.hpp"
#include "../protocol/loconet.hpp"
#include "../../enum/loconetserialinterface.hpp"
#include "../../enum/serialflowcontrol.hpp"
#include <boost/asio/serial_port.hpp>

namespace Hardware::CommandStation {

class LocoNetSerial : public CommandStation
{
  protected:
    boost::asio::serial_port m_serialPort;
    std::array<uint8_t, 1024> m_readBuffer;
    uint16_t m_readBufferOffset;

    bool setOnline(bool& value) final;
    void emergencyStopChanged(bool value) final;
    void trackVoltageOffChanged(bool value) final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    bool start();
    void stop();
    bool send(const Protocol::LocoNet::Message& msg);
    void read();

  public:
    CLASS_ID("hardware.command_station.loconet_serial")
    CREATE(LocoNetSerial)

    Property<std::string> port;
    Property<LocoNetSerialInterface> interface;
    Property<uint32_t> baudrate;
    Property<SerialFlowControl> flowControl;
    ObjectProperty<::Protocol::LocoNet> loconet;

    LocoNetSerial(const std::weak_ptr<World>& world, std::string_view _id);
};

}

#endif
