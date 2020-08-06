/**
 * server/src/hardware/commandstation/serialcommandstation.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_SERIALCOMMANDSTATION_HPP
#define TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_SERIALCOMMANDSTATION_HPP

#include "commandstation.hpp"
#include "../../enum/serialflowcontrol.hpp"
#include <boost/asio/serial_port.hpp>

class SerialCommandStation : public CommandStation
{
  protected:
    boost::asio::serial_port m_serialPort;
    std::array<uint8_t, 1024> m_readBuffer;
    uint16_t m_readBufferOffset;

    bool setOnline(bool& value) final;

    bool start();
    void stop();

    virtual void started() {}
    virtual void read() = 0;

  public:
    Property<std::string> port;
    Property<uint32_t> baudrate;
    Property<SerialFlowControl> flowControl;

    SerialCommandStation(const std::weak_ptr<World>& world, std::string_view _id);
};

#endif
