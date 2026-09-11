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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_IOHANDLER_CBUSSOCKETCANIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_IOHANDLER_CBUSSOCKETCANIOHANDLER_HPP

#include "cbusiohandler.hpp"
#include "../../can/iohandler/socketcaniohandler.hpp"

namespace CBUS {

// FIXME: merge this into CAN::SocketCANIOHandler so we have more generic CAN support
class SocketCANIOHandler : public IOHandler
{
public:
  SocketCANIOHandler(Kernel& kernel, const std::string& interface);

  void start() final;
  void stop() final;

  [[nodiscard]] std::error_code send(const CAN::Message& canMessage) final;

private:
  CAN::SocketCANIOHandler m_socketCAN;
};

}

#endif
