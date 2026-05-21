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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_KERNEL_HPP

#include "../kernelbase.hpp"
#include "config.hpp"
#include "protocol.hpp"
#include "../../output/outputvalue.hpp"

#include <functional>
#include <memory>
#include <thread>
#include <vector>
#include <cstdint>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>

namespace Marklin6050 {

class IOHandler;

class Kernel : public KernelBase
{
public:
  // Callbacks — set before start(); called on the EventLoop thread.
  std::function<void(uint32_t address, bool state)>                           s88Callback;
  std::function<void(bool power, bool run)>                                   extensionGlobalCallback;
  std::function<void(uint32_t address, bool green)>                           extensionTurnoutCallback;
  std::function<void(uint8_t address, uint8_t speed, bool f0, bool forward)>  extensionLocoCallback;
  std::function<void(uint8_t address, bool f1, bool f2, bool f3, bool f4)>    extensionFuncCallback;

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
  void setLocoFunctions1to4(uint8_t address, bool f1, bool f2, bool f3, bool f4);
  bool setAccessory(uint32_t address, OutputValue value, unsigned int timeMs);

  // IOHandler entry points — called on m_strand by IOHandler.
  void started() override;
  void receive(uint8_t byte);
  void readError(const boost::system::error_code& ec);
  void writeError(const boost::system::error_code& ec);

private:
  void sendRaw(uint8_t b1, uint8_t b2);
  void sendRaw(uint8_t b);
  void sendWithRedundancy(uint8_t b);
  void sendWithRedundancy(uint8_t b1, uint8_t b2);

  void scheduleS88Poll();
  void doS88Poll();

  enum class S88State { Idle, ReceivingData };
  S88State     m_s88State  = S88State::Idle;
  unsigned int m_s88Expect = 0;
  unsigned int m_s88Module = 0;
  uint8_t      m_s88High   = 0;

  void scheduleExtensionPoll();
  void doExtensionPoll();
  void processExtensionByte(uint8_t byte);
  void advanceExtensionEvent();

  enum class ExtState {
    Idle, WaitCount, WaitType, GlobalData,
    TurnoutAddr, TurnoutState,
    LocoStateAddr, LocoStateData,
    LocoFuncAddr, LocoFuncData
  };
  ExtState m_extState      = ExtState::Idle;
  uint8_t  m_extEventsLeft = 0;
  uint8_t  m_extTmpAddr    = 0;

  const Config                           m_config;
  boost::asio::io_context                m_ioContext;
  boost::asio::io_context::strand        m_strand;
  std::thread                            m_ioThread;
  std::unique_ptr<IOHandler>             m_ioHandler;
  boost::asio::steady_timer              m_s88Timer;
  boost::asio::steady_timer              m_extensionTimer;
  std::vector<boost::asio::steady_timer> m_redundancyTimers;
};

} // namespace Marklin6050

#endif
