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


#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_IOHANDLER_DINAMOIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_IOHANDLER_DINAMOIOHANDLER_HPP

#include <cstdint>
#include <span>
#include <system_error>
#include <boost/asio/steady_timer.hpp>

namespace Dinamo {

class Kernel;

class IOHandler
{
public:
  static constexpr size_t maxMessageSize = 39;

  IOHandler(const IOHandler&) = delete;
  IOHandler& operator =(const IOHandler&) = delete;

  virtual ~IOHandler() = default;

  void start();
  virtual void stop() = 0;

  [[nodiscard]] virtual std::error_code send(std::span<const uint8_t> message, bool hold, bool fault) = 0;

protected:
  Kernel& m_kernel;
  boost::asio::steady_timer m_timer;
  bool m_rxHold = false;
  bool m_rxFault = false;
  bool m_txHold = false;
  bool m_txFault = false;

  IOHandler(Kernel& kernel);

  void sendNull();

  void startIdleTimeoutTimer();
};

template<class T>
constexpr bool isSimulation()
{
  return false;
}

}

#endif
