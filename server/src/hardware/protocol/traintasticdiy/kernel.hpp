/**
 * server/src/hardware/protocol/traintasticdiy/kernel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICDIY_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICDIY_KERNEL_HPP

#include <unordered_map>
#include <thread>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <traintastic/enum/tristate.hpp>
#include "config.hpp"
#include "featureflags.hpp"
#include "inputstate.hpp"
#include "outputstate.hpp"
#include "iohandler/iohandler.hpp"

class InputController;
class OutputController;

namespace TraintasticDIY {

struct Message;

class Kernel
{
  private:
    boost::asio::io_context m_ioContext;
    std::unique_ptr<IOHandler> m_ioHandler;
    const bool m_simulation;
    std::thread m_thread;
    std::string m_logId;
    boost::asio::steady_timer m_heartbeatTimeout;
    std::function<void()> m_onStarted;

    bool m_featureFlagsSet;
    FeatureFlags1 m_featureFlags1;
    FeatureFlags2 m_featureFlags2;
    FeatureFlags3 m_featureFlags3;
    FeatureFlags4 m_featureFlags4;

    InputController* m_inputController;
    std::unordered_map<uint16_t, InputState> m_inputValues;

    OutputController* m_outputController;
    std::unordered_map<uint16_t, OutputState>  m_outputValues;

    Config m_config;
#ifndef NDEBUG
    bool m_started;
#endif

    Kernel(const Config& config, bool simulation);

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    template<class T>
    void postSend(const T& message)
    {
      m_ioContext.post(
        [this, message]()
        {
          send(message);
        });
    }

    void send(const Message& message);

    inline bool hasFeatureInput() const { return contains(m_featureFlags1, FeatureFlags1::Input); }
    inline bool hasFeatureOutput() const { return contains(m_featureFlags1, FeatureFlags1::Output); }

    void restartHeartbeatTimeout();
    void heartbeatTimeoutExpired(const boost::system::error_code& ec);

  public:
    static constexpr uint16_t ioAddressMin = 1;
    static constexpr uint16_t ioAddressMax = std::numeric_limits<uint16_t>::max();

    Kernel(const Kernel&) = delete;
    Kernel& operator =(const Kernel&) = delete;

    /**
     * \brief IO context for TraintasticDIY kernel and IO handler
     *
     * \return The IO context
     */
    boost::asio::io_context& ioContext() { return m_ioContext; }

    /**
     * \brief Create kernel and IO handler
     *
     * \param[in] config TraintasticDIY configuration
     * \param[in] args IO handler arguments
     * \return The kernel instance
     */
    template<class IOHandlerType, class... Args>
    static std::unique_ptr<Kernel> create(const Config& config, Args... args)
    {
      static_assert(std::is_base_of_v<IOHandler, IOHandlerType>);
      std::unique_ptr<Kernel> kernel{new Kernel(config, isSimulation<IOHandlerType>())};
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
     *
     *
     */
    inline const std::string& logId() { return m_logId; }

    /**
     * \brief Set object id used for log messages
     *
     * \param[in] value The object id
     */
    inline void setLogId(std::string value)
    {
      m_logId = std::move(value);
    }

    /**
     * \brief Set TraintasticDIY configuration
     *
     * \param[in] config The TraintasticDIY configuration
     */
    void setConfig(const Config& config);

    /**
     * \brief ...
     *
     * \param[in] callback ...
     * \note This function may not be called when the kernel is running.
     */
    inline void setOnStarted(std::function<void()> callback)
    {
      assert(!m_started);
      m_onStarted = std::move(callback);
    }

    /**
     * \brief Set the input controller
     *
     * \param[in] inputController The input controller
     * \note This function may not be called when the kernel is running.
     */
    inline void setInputController(InputController* inputController)
    {
      assert(!m_started);
      m_inputController = inputController;
    }

    /**
     * \brief Set the output controller
     *
     * \param[in] outputController The output controller
     * \note This function may not be called when the kernel is running.
     */
    inline void setOutputController(OutputController* outputController)
    {
      assert(!m_started);
      m_outputController = outputController;
    }

    /**
     * \brief Start the kernel and IO handler
     */
    void start();

    /**
     * \brief Stop the kernel and IO handler
     */
    void stop();

    /**
     * \brief ...
     *
     * This must be called by the IO handler whenever a TraintasticDIY message is received.
     *
     * \param[in] message The received TraintasticDIY message
     * \note This function must run in the kernel's IO context
     */
    void receive(const Message& message);

    /**
     *
     * \param[in] address Output address, #ioAddressMin..#ioAddressMax
     * \param[in] value Output value: \c true is on, \c false is off.
     * \return \c true if send successful, \c false otherwise.
     */
    bool setOutput(uint16_t address, bool value);

    /**
     * \brief Simulate input change
     * \param[in] address Input address, #ioAddressMin..#ioAddressMax
     */
    void simulateInputChange(uint16_t address);
};

}

#endif
