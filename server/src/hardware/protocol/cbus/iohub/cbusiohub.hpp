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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_IOHUB_CBUSIOHUB_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_IOHUB_CBUSIOHUB_HPP

#include "../../can/iohub/caniohub.hpp"

namespace CBUS {

class IOHub final : public CAN::IOHub
{
public:
  IOHub(boost::asio::io_context& ioContext, std::string logId, bool localhostOnly, uint16_t port);

  IOHub(const IOHub&) = delete;
  IOHub& operator =(const IOHub&) = delete;

protected:
  std::shared_ptr<CAN::IOHubConnection> newConnection(boost::asio::ip::tcp::socket socket) final;
};

}

#endif
