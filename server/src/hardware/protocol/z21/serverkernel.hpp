/**
 * server/src/hardware/protocol/z21/serverkernel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_SERVERKERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_SERVERKERNEL_HPP

#include "kernel.hpp"
#include <list>
#include <map>
#include <unordered_map>
#include <boost/asio/steady_timer.hpp>
#include <boost/signals2/signal.hpp>
#include <traintastic/enum/tristate.hpp>
#include "messages.hpp"

class DecoderList;

namespace Z21 {

class ServerKernel final : public Kernel
{
  private:
    struct Client
    {
      std::chrono::time_point<std::chrono::steady_clock> lastSeen;
      BroadcastFlags broadcastFlags = BroadcastFlags::None;
      std::list<std::pair<uint16_t, bool>> subscriptions;
    };

    struct DecoderSubscription
    {
      boost::signals2::connection connection;
      size_t count; //!< number of clients subscribed to the decoder
    };

    boost::asio::steady_timer m_inactiveClientPurgeTimer;
    ServerConfig m_config;
    std::shared_ptr<DecoderList> m_decoderList;
    std::unordered_map<IOHandler::ClientId, Client> m_clients;
    std::map<std::pair<uint16_t, bool>, DecoderSubscription> m_decoderSubscriptions;
    TriState m_trackPowerOn = TriState::Undefined;
    std::function<void()> m_onTrackPowerOff;
    std::function<void()> m_onTrackPowerOn;
    TriState m_emergencyStop = TriState::Undefined;
    std::function<void()> m_onEmergencyStop;

    ServerKernel(std::string logId_, const ServerConfig& config, std::shared_ptr<DecoderList> decoderList);

    void onStart() final;
    void onStop() final;

    template<class T>
    void postSendTo(const T& message, IOHandler::ClientId clientId)
    {
      m_ioContext.post(
        [this, message, clientId]()
        {
          sendTo(message, clientId);
        });
    }

    void sendTo(const Message& message, IOHandler::ClientId clientId);
    void sendTo(const Message& message, BroadcastFlags broadcastFlags);

    LanSystemStateDataChanged getLanSystemStateDataChanged() const;

    std::shared_ptr<Decoder> getDecoder(uint16_t address, bool longAddress) const;

    void removeClient(IOHandler::ClientId clientId);
    void subscribe(IOHandler::ClientId clientId, uint16_t address, bool longAddress);
    void unsubscribe(IOHandler::ClientId clientId, std::pair<uint16_t, bool> key);
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);

    void startInactiveClientPurgeTimer();
    void inactiveClientPurgeTimerExpired(const boost::system::error_code& ec);

  public:
    /**
     * @brief Create kernel and IO handler
     * @param[in] config Z21 server configuration
     * @param[in] args IO handler arguments
     * @return The kernel instance
     */
    template<class IOHandlerType, class... Args>
    static std::unique_ptr<ServerKernel> create(std::string logId_, const ServerConfig& config, std::shared_ptr<DecoderList> decoderList, Args... args)
    {
      static_assert(std::is_base_of_v<IOHandler, IOHandlerType>);
      std::unique_ptr<ServerKernel> kernel{new ServerKernel(std::move(logId_), config, std::move(decoderList))};
      kernel->setIOHandler(std::make_unique<IOHandlerType>(*kernel, std::forward<Args>(args)...));
      return kernel;
    }

    /**
     * @brief ...
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    inline void setOnTrackPowerOff(std::function<void()> callback)
    {
      assert(!m_started);
      m_onTrackPowerOff = std::move(callback);
    }

    /**
     * @brief ...
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    inline void setOnTrackPowerOn(std::function<void()> callback)
    {
      assert(!m_started);
      m_onTrackPowerOn = std::move(callback);
    }

    /**
     * @brief ...
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    inline void setOnEmergencyStop(std::function<void()> callback)
    {
      assert(!m_started);
      m_onEmergencyStop = std::move(callback);
    }

    /**
     * @brief Set Z21 server configuration
     * @param[in] config The Z21 server configuration
     */
    void setConfig(const ServerConfig& config);

    /**
     * @brief Set Z21 state
     * @param[in] trackPowerOn \c true if track power is on, \c false if track power is off
     * @param[in] emergencyStop \c true if emergency stop is active, \c false if emergency stop isn't active
     */
    void setState(bool trackPowerOn, bool emergencyStop);

    /**
     * @brief Incoming message handler
     * This method must be called by the IO handler whenever a Z21 message is received.
     * @param[in] message The received Z21 message
     * @param[in] cliendId The client who sent the message
     * @note This function must run in the kernel's IO context
     */
    void receiveFrom(const Message& message, IOHandler::ClientId clientId);
};

}

#endif
