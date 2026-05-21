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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6023_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6023_KERNEL_HPP

#include "../kernelbase.hpp"
#include "config.hpp"
#include "../../output/outputvalue.hpp"

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <cstdint>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>

namespace Marklin6023 {

class IOHandler;

class Kernel : public KernelBase
{
public:
  // Callbacks — set before start(); called on the EventLoop thread.
  std::function<void(uint32_t address, bool state)> s88Callback;

  /**
   * logId_ is passed to KernelBase (differs from the inherited logId member
   * to avoid -Wshadow).
   */
  Kernel(std::string logId_, const Config& config);

  /** Declared here so ~unique_ptr<IOHandler> resolves only in kernel.cpp. */
  ~Kernel() override;

  /** Called by the interface before start(). */
  void setIOHandler(std::unique_ptr<IOHandler> handler);

  /** Accessors used by IOHandler subclasses. */
  boost::asio::io_context&         ioContext() { return m_ioContext; }
  boost::asio::io_context::strand& strand()    { return m_strand; }

  void start();
  void stop();

  // Command API — thread-safe, may be called from any thread.
  void sendGlobalGo();
  void sendGlobalStop();
  void setLocoSpeed(uint8_t address, uint8_t speed, bool f0);
  void setLocoDirection(uint8_t address, bool f0);
  void setLocoEmergencyStop(uint8_t address, bool f0);
  void setLocoFunction(uint8_t address, uint8_t currentSpeed, bool f0);
  bool setAccessory(uint32_t address, OutputValue value);

  // IOHandler entry points — called on m_strand by IOHandler.
  void started() override;
  void receiveLine(std::string line);
  void readError(const boost::system::error_code& ec);
  void writeError(const boost::system::error_code& ec);

private:
  void sendCmd(std::string cmd);
  void sendCmdWithRedundancy(std::string cmd);

  void startS88Cycle();
  void queryNextContact();
  void onS88Response(const std::string& line);
  void onS88ResponseTimeout();

  const Config                           m_config;
  boost::asio::io_context                m_ioContext;
  boost::asio::io_context::strand        m_strand;
  std::thread                            m_ioThread;

  unsigned int m_s88NextContact  = 1;
  bool         m_s88WaitingReply = false;
  uint32_t     m_s88LastQueried  = 0;

  std::unique_ptr<IOHandler>             m_ioHandler;
  boost::asio::steady_timer              m_s88Timer;
  boost::asio::steady_timer              m_s88ResponseTimer;
  std::vector<boost::asio::steady_timer> m_redundancyTimers;
};

} // namespace Marklin6023

#endif
