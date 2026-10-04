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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_IOHUB_CBUSIOHUBCONNECTION_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_IOHUB_CBUSIOHUBCONNECTION_HPP

#include "../../can/iohub/caniohubconnection.hpp"

namespace CBUS {

class IOHubConnection final : public CAN::IOHubConnection
{
public:
  IOHubConnection(std::shared_ptr<CAN::IOHub> hub, boost::asio::ip::tcp::socket socket);

  IOHubConnection(const IOHubConnection&) = delete;
  IOHubConnection& operator =(const IOHubConnection&) = delete;

protected:
  size_t deserialize(std::span<const std::byte> buffer, CAN::Message& message, bool& haveMessage) final;
  size_t serialize(const CAN::Message& message, std::span<std::byte> buffer) final;
};

}

#endif
