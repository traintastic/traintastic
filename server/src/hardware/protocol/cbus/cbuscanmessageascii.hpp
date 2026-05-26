/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSCANMESSAGEASCII_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSCANMESSAGEASCII_HPP

#include <span>
#include <string_view>
#include "../can/canmessage.hpp"

namespace CBUS {

/**
 * \brief Serialize CAN::Message to CBUS's GridConnect flavored ASCII format.
 * \return Number of chars written to buffer, \c 0 on error.
 */
std::size_t toAscii(const CAN::Message& canMessage, std::span<char> buffer);

/**
 * \brief Deserialize CAN::Message from CBUS's GridConnect flavored ASCII format.
 *
 * \return Number of chars consumed from the buffer.
 */
std::size_t fromAscii(std::string_view buffer, CAN::Message& canMessage, std::size_t& dropped);

}

#endif
