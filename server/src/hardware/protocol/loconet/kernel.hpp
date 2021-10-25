/**
 * server/src/hardware/protocol/loconet/kernel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_KERNEL_HPP

#include <array>
#include <thread>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <traintastic/enum/tristate.hpp>
#include "config.hpp"
#include "iohandler/iohandler.hpp"

class Decoder;
enum class DecoderChangeFlags;
class DecoderController;
class InputController;
class OutputController;

namespace LocoNet {

struct Message;

class Kernel
{
  private:
    enum Priority
    {
      HighPriority = 0,
      NormalPriority = 1,
      LowPriority = 2,
    };

    class SendQueue
    {
      private:
        std::array<std::byte, 4000> m_buffer;
        std::byte* m_front;
        std::size_t m_bytes;

        constexpr std::size_t threshold() const noexcept { return m_buffer.size() / 2; }

      public:
        SendQueue()
          : m_front{m_buffer.data()}
          , m_bytes{0}
        {
        }

        inline bool empty() const
        {
          return m_bytes == 0;
        }

        inline const Message& front() const
        {
          return *reinterpret_cast<const Message*>(m_front);
        }

        bool append(const Message& message);

        void pop();
    };

    struct LocoSlot
    {
      uint16_t address = 0;
      bool addressValid = false;
      uint8_t speed = 0;
      bool speedValid = false;
      uint8_t dirf0f4 = 0;
      bool dirf0f4Valid = false;
      uint8_t f5f8 = 0;
      bool f5f8Valid = false;
      uint8_t f9f12 = 0;
      bool f9f12Valid = false;
      uint8_t f13f19 = 0;
      bool f13f19Valid = false;
      uint8_t f21f27 = 0;
      bool f21f27Valid = false;
      uint8_t f20f28 = 0;
      bool f20f28Valid = false;

      void invalidate()
      {
        addressValid = false;
        speedValid = false;
        dirf0f4Valid = false;
        f5f8Valid = false;
        f9f12Valid = false;
        f13f19Valid = false;
        f20f28Valid = false;
        f21f27Valid = false;
      }
    };

    boost::asio::io_context m_ioContext;
    std::unique_ptr<IOHandler> m_ioHandler;
    std::thread m_thread;
    std::string m_logId;
    std::function<void()> m_onStarted;

    std::array<SendQueue, 3> m_sendQueue;
    Priority m_sentMessagePriority;
    bool m_waitingForEcho;

    TriState m_globalPower;
    std::function<void(bool)> m_onGlobalPowerChanged;

    TriState m_emergencyStop;
    std::function<void()> m_onIdle;

    boost::asio::steady_timer m_fastClockSyncTimer;

    DecoderController* m_decoderController;
    std::unordered_map<uint16_t, uint8_t> m_addressToSlot;
    std::unordered_map<uint8_t, LocoSlot> m_slots;
    std::unordered_map<uint16_t, std::vector<std::byte>> m_pendingSlotMessages;

    InputController* m_inputController;
    std::array<TriState, 4096> m_inputValues;

    OutputController* m_outputController;
    std::array<TriState, 4096> m_outputValues;

    Config m_config;
#ifndef NDEBUG
    bool m_started;
#endif

    Kernel(const Config& config);

    LocoSlot* getLocoSlot(uint8_t slot, bool sendSlotDataRequestIfNew = true);

    std::shared_ptr<Decoder> getDecoder(uint16_t address);

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    void send(const Message& message, Priority priority = NormalPriority);
    void send(uint16_t address, Message& message, uint8_t& slot);
    template<typename T>
    inline void send(uint16_t address, T& message)
    {
      send(address, message, message.slot);
    }
    void sendNextMessage();

    void startFastClockSyncTimer();
    void stopFastClockSyncTimer();
    void fastClockSyncTimerExpired(const boost::system::error_code& ec);

  public:
    static constexpr uint16_t inputAddressMin = 1;
    static constexpr uint16_t inputAddressMax = 4096;
    static constexpr uint16_t outputAddressMin = 1;
    static constexpr uint16_t outputAddressMax = 4096;

    Kernel(const Kernel&) = delete;
    Kernel& operator =(const Kernel&) = delete;

    /**
     * @brief IO context for LocoNet kernel and IO handler
     *
     * @return The IO context
     */
    boost::asio::io_context& ioContext() { return m_ioContext; }

    /**
     * @brief Create kernel and IO handler
     *
     * @param[in] config LocoNet configuration
     * @param[in] args IO handler arguments
     * @return The kernel instance
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
     * @brief Access the IO handler
     *
     * @return The IO handler
     * @note The IO handler runs in the kernel's IO context, not all functions can be called safely!
     */
    template<class T>
    T& ioHandler()
    {
      assert(dynamic_cast<T*>(m_ioHandler.get()));
      return static_cast<T&>(*m_ioHandler);
    }

    /**
     * @brief Set object id used for log messages
     *
     * @param[in] value The object id
     */
    void setLogId(std::string value) { m_logId = std::move(value); }

    /**
     * @brief Set LocoNet configuration
     *
     * @param[in] config The LocoNet configuration
     */
    void setConfig(const Config& config);

    /**
     * @brief ...
     *
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    void setOnStarted(std::function<void()> callback);

    /**
     * @brief ...
     *
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    void setOnGlobalPowerChanged(std::function<void(bool)> callback);

    /**
     * @brief ...
     *
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    void setOnIdle(std::function<void()> callback);

    /**
     * @brief Set the decoder controller
     *
     * @param[in] decoderController The decoder controller
     * @note This function may not be called when the kernel is running.
     */
    void setDecoderController(DecoderController* decoderController);

    /**
     * @brief Set the input controller
     *
     * @param[in] inputController The input controller
     * @note This function may not be called when the kernel is running.
     */
    void setInputController(InputController* inputController);

    /**
     * @brief Set the output controller
     *
     * @param[in] outputController The output controller
     * @note This function may not be called when the kernel is running.
     */
    void setOutputController(OutputController* outputController);

    /**
     * @brief Start the kernel and IO handler
     */
    void start();

    /**
     * @brief Stop the kernel and IO handler
     */
    void stop();

    /**
     * @brief ...
     *
     * This must be called by the IO handler whenever a LocoNet message is received.
     *
     * @param[in] message The received LocoNet message
     * @note This function must run in the kernel's IO context
     */
    void receive(const Message& message);

    /**
     *
     *
     */
    void setPowerOn(bool value);

    /**
     *
     *
     */
    void emergencyStop();


    /**
     *
     */
    void resume();

    //TriState getInput(uint16_t address) const;

    //TriState getOutput(uint16_t address) const;

    /**
     *
     *
     */
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);

    /**
     *
     * @param[in] address Output address, 1..4096
     * @param[in] value Output value: \c true is on, \c false is off.
     * @return \c true if send successful, \c false otherwise.
     */
    bool setOutput(uint16_t address, bool value);
};

}

#endif
