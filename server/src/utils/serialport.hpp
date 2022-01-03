/**
 * server/src/utils/serialport.hpp
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

#ifndef TRAINTASTIC_SERVER_UTILS_SERIALPORT_HPP
#define TRAINTASTIC_SERVER_UTILS_SERIALPORT_HPP

#include <cstdint>
#include <array>
#include <boost/asio/serial_port.hpp>
#include <traintastic/enum/serialparity.hpp>
#include <traintastic/enum/serialstopbits.hpp>
#include <traintastic/enum/serialflowcontrol.hpp>

namespace SerialPort {

constexpr uint32_t baudrateMin = 50;
constexpr uint32_t baudrateMax = 3'000'000;
constexpr std::array<uint32_t, 14> baudrateValues = {{110, 300, 600, 1'200, 2'400, 4'800, 9'600, 14'400, 19'200, 38'400, 57'600, 115'200, 128'000, 256'000}};

/**
 * @brief Open serial port
 *
 * @note Throws LogMessageException on failure
 */
void open(boost::asio::serial_port& serialPort, const std::string& device, uint32_t baudrate, uint8_t characterSize, SerialParity parity, SerialStopBits stopBits, SerialFlowControl flowControl);

}

#endif
