/**
 * server/src/hardware/protocol/z21/clientkernel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_CLIENTKERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_CLIENTKERNEL_HPP

#include <unordered_map>

#include "kernel.hpp"
#include <boost/asio/steady_timer.hpp>
#include <traintastic/enum/outputchannel.hpp>
#include <traintastic/enum/tristate.hpp>
#include "../../output/outputvalue.hpp"

enum class SimulateInputAction;
class InputController;
class OutputController;

namespace Z21 {

class ClientKernel final : public Kernel
{
  public:
    static constexpr uint32_t outputAddressMin = 1;
    static constexpr uint32_t outputAddressMax = 4096;
    static constexpr uint16_t rbusAddressMin = 1;
    static constexpr uint16_t rbusAddressMax = 1000; //!< \todo what is the maximum
    static constexpr uint16_t loconetAddressMin = 1;
    static constexpr uint16_t loconetAddressMax = 4096;

    struct InputChannel
    {
      static constexpr uint32_t rbus = 1;
      static constexpr uint32_t loconet = 2;
    };

    inline static const std::vector<uint32_t> inputChannels = {
      InputChannel::rbus,
      InputChannel::loconet,
    };

    inline static const std::vector<std::string_view> inputChannelNames = {
      "$z21_channel:rbus$",
      "$hardware:loconet$",
    };

  private:
    const bool m_simulation;
    boost::asio::steady_timer m_keepAliveTimer;
    boost::asio::steady_timer m_inactiveDecoderPurgeTimer;
    BroadcastFlags m_broadcastFlags;
    int m_broadcastFlagsRetryCount;
    static constexpr int maxBroadcastFlagsRetryCount = 10;

    static constexpr BroadcastFlags requiredBroadcastFlags =
      BroadcastFlags::PowerLocoTurnoutChanges |
      BroadcastFlags::RBusChanges |
      BroadcastFlags::SystemStatusChanges |
      BroadcastFlags::AllLocoChanges | // seems not to work with DR5000
      BroadcastFlags::LocoNetDetector;

    uint32_t m_serialNumber;
    std::function<void(uint32_t)> m_onSerialNumberChanged;

    HardwareType m_hardwareType;
    uint8_t m_firmwareVersionMajor;
    uint8_t m_firmwareVersionMinor;
    std::function<void(HardwareType, uint8_t, uint8_t)> m_onHardwareInfoChanged;

    /*!
     * \brief m_trackPowerOn caches command station track power state.
     *
     * \note It must be accessed only from event loop thread or from
     * Z21::ClientKernel::onStart().
     *
     * \sa EventLoop
     */
    TriState m_trackPowerOn = TriState::Undefined;

    /*!
     * \brief m_emergencyStop caches command station emergency stop state.
     *
     * \note It must be accessed only from event loop thread or from
     * Z21::ClientKernel::onStart().
     *
     * \sa EventLoop
     */
    TriState m_emergencyStop = TriState::Undefined;

    /*!
     * \brief m_onTrackPowerChanged callback is called when Z21 power state changes.
     *
     * \note It is always called from event loop thread
     * \note First argument is powerOn, second argument is isStopped
     * In Z21 EmergencyStop is really PowerOn + EmergencyStop and
     * PowerOn implicitly means Run so we cannot call \sa trackPowerOn() if world must be stopped
     *
     * \sa EventLoop
     */
    std::function<void(bool, bool)> m_onTrackPowerChanged;

    DecoderController* m_decoderController = nullptr;

    struct LocoCache
    {
      enum class Trend : bool
      {
          Ascending = 0,
          Descending
      };

      uint16_t dccAddress = 0;
      bool isEStop = false;
      uint8_t speedStep = 0;
      uint8_t speedSteps = 0;
      uint8_t lastReceivedSpeedStep = 0; //Always in 126 steps
      Trend speedTrend = Trend::Ascending;
      bool speedTrendExplicitlySet = false;
      Direction direction = Direction::Unknown;
      std::chrono::steady_clock::time_point lastSetTime;
    };

    std::unordered_map<uint16_t, LocoCache> m_locoCache;
    bool m_isUpdatingDecoderFromKernel = false;

    InputController* m_inputController = nullptr;
    std::array<TriState, rbusAddressMax - rbusAddressMin + 1> m_rbusFeedbackStatus;
    std::array<TriState, loconetAddressMax - loconetAddressMin + 1> m_loconetFeedbackStatus;

    OutputController* m_outputController = nullptr;

    ClientConfig m_config;

    ClientKernel(std::string logId_, const ClientConfig& config, bool simulation);

    void onStart() final;
    void onStop() final;

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

    void startKeepAliveTimer();
    void keepAliveTimerExpired(const boost::system::error_code& ec);

    void startInactiveDecoderPurgeTimer();
    void inactiveDecoderPurgeTimerExpired(const boost::system::error_code &ec);

    LocoCache &getLocoCache(uint16_t dccAddr);

public:
    /**
     * @brief Create kernel and IO handler
     * @param[in] config Z21 client configuration
     * @param[in] args IO handler arguments
     * @return The kernel instance
     */
    template<class IOHandlerType, class... Args>
    static std::unique_ptr<ClientKernel> create(std::string logId_, const ClientConfig& config, Args... args)
    {
      static_assert(std::is_base_of_v<IOHandler, IOHandlerType>);
      std::unique_ptr<ClientKernel> kernel{new ClientKernel(std::move(logId_), config, isSimulation<IOHandlerType>())};
      kernel->setIOHandler(std::make_unique<IOHandlerType>(*kernel, std::forward<Args>(args)...));
      return kernel;
    }

    /**
     * @brief Set Z21 client configuration
     * @param[in] config The Z21 client configuration
     */
    void setConfig(const ClientConfig& config);

    /**
     * @brief Incoming message handler
     * This method must be called by the IO handler whenever a Z21 message is received.
     * @param[in] message The received Z21 message
     * @note This function must run in the kernel's IO context
     */
    void receive(const Message& message);

    /**
     * @brief ...
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    inline void setOnSerialNumberChanged(std::function<void(uint32_t)> callback)
    {
      assert(!m_started);
      m_onSerialNumberChanged = std::move(callback);
    }

    /**
     * @brief ...
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    inline void setOnHardwareInfoChanged(std::function<void(HardwareType, uint8_t, uint8_t)> callback)
    {
      assert(!m_started);
      m_onHardwareInfoChanged = std::move(callback);
    }

    /**
     * @brief ...
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    inline void setOnTrackPowerChanged(std::function<void(bool, bool)> callback)
    {
      assert(!m_started);
      m_onTrackPowerChanged = std::move(callback);
    }

    /**
     */
    void trackPowerOn();

    /**
     */
    void trackPowerOff();

    /**
     */
    void emergencyStop();

    /**
     * @brief Set the decoder controller
     * @param[in] decoderController The decoder controller
     * @note This function may not be called when the kernel is running.
     */
    inline void setDecoderController(DecoderController* decoderController)
    {
      assert(!m_started);
      m_decoderController = decoderController;
    }

    /**
     * @brief Set the input controller
     *
     * @param[in] inputController The input controller
     * @note This function may not be called when the kernel is running.
     */
    void setInputController(InputController* inputController)
    {
      assert(!m_started);
      m_inputController = inputController;
    }

    /**
     * @brief Set the output controller
     *
     * @param[in] outputController The output controller
     * @note This function may not be called when the kernel is running.
     */
    void setOutputController(OutputController* outputController)
    {
      assert(!m_started);
      m_outputController = outputController;
    }

    /**
     */
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);

    /**
     *
     * @param[in] address Output address, \ref outputAddressMin .. \ref outputAddressMax
     * @param[in] value Output value: \c true is on, \c false is off.
     * @return \c true if send successful, \c false otherwise.
     */
    bool setOutput(OutputChannel channel, uint16_t address, OutputValue value);

    void simulateInputChange(uint32_t channel, uint32_t address, SimulateInputAction action);
};

}

#endif
