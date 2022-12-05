/**
 * server/src/hardware/protocol/withrottle/kernel.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_WITHROTTLE_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_WITHROTTLE_KERNEL_HPP

#include <thread>
#include <unordered_map>
#include <boost/asio/io_context.hpp>
#include <boost/signals2/connection.hpp>
#include <traintastic/enum/tristate.hpp>
#include "config.hpp"
#include "iohandler/iohandler.hpp"

class Clock;
class HardwareThrottle;
class ThrottleController;

namespace WiThrottle {

enum class ThrottleCommand : char;
struct Address;

class Kernel
{
  private:
    struct MultiThrottle
    {
      uint16_t address;
      bool isLongAddress;
      std::shared_ptr<HardwareThrottle> throttle;
    };

    struct Client
    {
      std::string id;
      std::string name;
      std::unordered_map<char, MultiThrottle> multiThrottles;
    };

    boost::asio::io_context m_ioContext;
    std::unique_ptr<IOHandler> m_ioHandler;
    std::thread m_thread;
    std::string m_logId;
    std::function<void()> m_onStarted;

    TriState m_powerOn;

    std::shared_ptr<Clock> m_clock;
    boost::signals2::connection m_clockChangeConnection;

    ThrottleController* m_throttleController;
    std::unordered_map<IOHandler::ClientId, Client> m_clients;

    Config m_config;
    bool m_running = false;

    Kernel(const Config& config);

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    void postSendTo(std::string message, IOHandler::ClientId clientId)
    {
      m_ioContext.post(
        [this, msg=std::move(message), clientId]()
        {
          sendTo(msg, clientId);
        });
    }

    void postSendToAll(std::string message)
    {
      m_ioContext.post(
        [this, msg=std::move(message)]()
        {
          sendToAll(msg);
        });
    }

    void sendTo(std::string_view message, IOHandler::ClientId clientId);
    void sendToAll(std::string_view message);

    MultiThrottle* getMultiThrottle(IOHandler::ClientId clientId, char multiThrottleId);
    const std::shared_ptr<HardwareThrottle>& getThottle(IOHandler::ClientId clientId, char multiThrottleId = invalidMultiThrottleId);

    void multiThrottleAction(IOHandler::ClientId clientId, char multiThrottleId, const Address& address, ThrottleCommand throttleCommand, std::string_view message);

    void throttleReleased(IOHandler::ClientId clientId, char multiThrottleId);

  public:
    static constexpr char invalidMultiThrottleId = '\0';

    Kernel(const Kernel&) = delete;
    Kernel& operator =(const Kernel&) = delete;

#ifndef NDEBUG
    bool isKernelThread() const
    {
      return std::this_thread::get_id() == m_thread.get_id();
    }
#endif

    /**
     * \brief IO context for WiThrottle kernel and IO handler
     *
     * \return The IO context
     */
    boost::asio::io_context& ioContext() { return m_ioContext; }

    /**
     * \brief Create kernel and IO handler
     *
     * \param[in] args IO handler arguments
     * \return The kernel instance
     */
    template<class IOHandlerType, class... Args>
    static std::unique_ptr<Kernel> create(const Config& config, Args... args)
    {
      static_assert(std::is_base_of_v<IOHandler, IOHandlerType>);
      std::unique_ptr<Kernel> kernel{new Kernel(config)};
      kernel->setIOHandler(std::make_unique<IOHandlerType>(*kernel, std::forward<Args>(args)...));
      return kernel;
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
     * \brief Set WiThrottle configuration
     *
     * \param[in] config The WiThrottle configuration
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
      assert(!m_running);
      m_onStarted = std::move(callback);
    }

    /**
     * \brief Set clock for LocoNet fast clock
     *
     * \param[in] clock The clock
     * \note This function may not be called when the kernel is running.
     */
    void setClock(std::shared_ptr<Clock> clock);

    /**
     * \brief Set the throttle controller
     *
     * \param[in] throttleController The throttle controller
     * \note This function may not be called when the kernel is running.
     */
    inline void setThrottleController(ThrottleController* throttleController)
    {
      assert(!m_running);
      m_throttleController = throttleController;
    }

    /**
     *
     */
    void start();

    /**
     *
     */
    void stop();

    /**
     *
     *
     */
    void setPowerOn(bool on);

    /**
     * \brief New client handler
     * \param[in] cliendId The client who connected
     * \note This function must run in the kernel's IO context
     */
    void newClient(IOHandler::ClientId clientId);

    /**
     * \brief Client gone handler
     * \param[in] cliendId The client who disconnected
     * \note This function must run in the kernel's IO context
     */
    void clientGone(IOHandler::ClientId clientId);

    /**
     * \brief Incoming message handler
     * This method must be called by the IO handler whenever a WiThrottle message is received.
     * \param[in] message The received WiThrottle message
     * \param[in] cliendId The client who sent the message
     * \note This function must run in the kernel's IO context
     */
    void receiveFrom(std::string_view message, IOHandler::ClientId clientId);
};

}

#endif
