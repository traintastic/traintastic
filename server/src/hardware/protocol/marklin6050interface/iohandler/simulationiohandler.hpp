/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Kamil Kasprzak
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_IOHANDLER_SIMULATIONIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_IOHANDLER_SIMULATIONIOHANDLER_HPP

#include "iohandler.hpp"

#include <boost/asio/io_context.hpp>

namespace Marklin6050 {

class SimulationIOHandler final : public IOHandler
{
public:
  SimulationIOHandler(Kernel& kernel,
                      boost::asio::io_context::strand& strand);

  void start() final;
  void stop() final;
  void send(std::initializer_list<uint8_t> bytes) final;

private:
  boost::asio::io_context::strand& m_strand;
};

template<>
constexpr bool isSimulation<SimulationIOHandler>()
{
  return true;
}

} // namespace Marklin6050

#endif
