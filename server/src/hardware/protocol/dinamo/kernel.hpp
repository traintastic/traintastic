/**
 * server/src/hardware/protocol/dinamo/kernel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_KERNEL_HPP

#include "../kernelbase.hpp"
#include <boost/asio/steady_timer.hpp>
#include <traintastic/enum/tristate.hpp>
#include "config.hpp"
#include "iohandler/iohandler.hpp"

namespace Dinamo {

class Kernel : public ::KernelBase
{
public:
  struct ProtocolVersion
  {
    static const ProtocolVersion minimum;

    uint8_t major = 0;
    uint8_t minor = 0;
    uint8_t sub = 0;
    uint8_t bugFix = 0;

    std::string toString() const;
  };

  using OnFault = std::function<void()>;

private:
  //! Startup states, executed in order.
  enum class State
  {
    Initial, // must be first
    ProtocolVersion,
    Started // must be last
  };

  std::unique_ptr<IOHandler> m_ioHandler;
  const bool m_simulation;
  State m_state = State::Initial;
  boost::asio::steady_timer m_keepAliveTimeout;
  ProtocolVersion m_protocolVersion;
  bool m_sendToggle = false;
  TriState m_fault = TriState::Undefined;
  OnFault m_onFault;
  Config m_config;

  Kernel(std::string logId, const Config& config, bool simulation);

  void setIOHandler(std::unique_ptr<IOHandler> handler);

  void send(Message& message);
  void restartKeepAliveTimeout();

  void changeState(State value);
  inline void nextState()
  {
    assert(m_state != State::Started);
    changeState(static_cast<State>(static_cast<std::underlying_type_t<State>>(m_state) + 1));
  }

public:
  Kernel(const Kernel&) = delete;
  Kernel& operator =(const Kernel&) = delete;

#ifndef NDEBUG
  bool isKernelThread() const
  {
    return std::this_thread::get_id() == m_thread.get_id();
  }
#endif

  /**
   * \brief Create kernel and IO handler
   *
   * \param[in] logId_ Interface id to use for logging
   * \param[in] config Configuration options
   * \param[in] args IO handler arguments
   * \return The kernel instance
   */
  template<class IOHandlerType, class... Args>
  static std::unique_ptr<Kernel> create(std::string logId_, const Config& config, Args... args)
  {
    static_assert(std::is_base_of_v<IOHandler, IOHandlerType>);
    std::unique_ptr<Kernel> kernel{new Kernel(std::move(logId_), config, isSimulation<IOHandlerType>())};
    kernel->setIOHandler(std::make_unique<IOHandlerType>(*kernel, std::forward<Args>(args)...));
    return kernel;
  }

  /**
   * \brief Access the IO handler
   *
   * \return The IO handler
   * \note The IO handler runs in the kernel's IO context, not all functions can be called safely!
   */
  template<class T>
  T& ioHandler()
  {
    assert(dynamic_cast<T*>(m_ioHandler.get()));
    return static_cast<T&>(*m_ioHandler);
  }

  /**
   * \brief Set configuration
   *
   * \param[in] config The configuration
   */
  void setConfig(const Config& config);

  void setOnFault(OnFault callback);

  /**
   * \brief Start the kernel and IO handler
   */
  void start();

  /**
   * \brief Stop the kernel and IO handler
   */
  void stop();

  /**
   * \brief Notify kernel the IO handler is started.
   * \note This function must run in the kernel's IO context
   */
  void started() final;

  /**
   * \brief ...
   *
   * This must be called by the IO handler whenever a message is received.
   *
   * \param[in] message The received message
   * \note This function must run in the kernel's IO context
   */
  void receive(const Message& message);

  void setFault();
  void resetFault();
};

}

#endif
