/**
 * server/src/hardware/protocol/loconet/kernel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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

#include "../kernelbase.hpp"
#include <array>
#include <unordered_map>
#include <filesystem>
#include <boost/asio/steady_timer.hpp>
#include <boost/signals2/connection.hpp>
#include <span>
#include <traintastic/enum/direction.hpp>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/enum/outputchannel.hpp>
#include "config.hpp"
#include "iohandler/iohandler.hpp"
#include "../../output/outputvalue.hpp"

class Clock;
class Decoder;
enum class DecoderChangeFlags;
class DecoderController;
enum class SimulateInputAction;
class InputController;
class OutputController;
class IdentificationController;
class PCAP;

namespace LocoNet {

struct Message;

class Kernel : public ::KernelBase
{
  public:
    using OnLNCVReadResponse = std::function<void(bool, uint16_t, uint16_t)>;

    static constexpr uint16_t accessoryOutputAddressMin = 1;
    static constexpr uint16_t accessoryOutputAddressMax = 2048;

  private:
    enum Priority
    {
      HighPriority = 0,
      NormalPriority = 1,
      LowPriority = 2,
    };
    friend constexpr Priority& operator ++(Priority& value);

    class SendQueue
    {
      private:
        std::array<std::byte, 4000> m_buffer;
        std::byte* m_front;
        std::size_t m_bytes;

        constexpr std::size_t threshold() const noexcept { return m_buffer.size() / 2; }

      public:
        SendQueue()
          : m_buffer{}
          , m_front{m_buffer.data()}
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

        void clear();
    };

    struct LocoSlot
    {
      static constexpr uint16_t invalidAddress = 0xFFFF;
      static constexpr uint8_t invalidSpeed = 0xFF;

      uint16_t address;
      uint8_t speed;
      Direction direction;
      std::array<TriState, 29> functions;

      LocoSlot()
      {
        invalidate();
      }

      bool isAddressValid() const
      {
        return address != invalidAddress;
      }

      void invalidate()
      {
        address = invalidAddress;
        speed = invalidSpeed;
        direction = Direction::Unknown;
        for(auto& f : functions)
          f = TriState::Undefined;
      }
    };

    struct FastClock
    {
      uint8_t multiplier;
      uint8_t hour;
      uint8_t minute;
      uint8_t : 8; // padding for std::atomic
    };
    static_assert(sizeof(FastClock) == 4);

    std::unique_ptr<IOHandler> m_ioHandler;
    const bool m_simulation;

    std::array<SendQueue, 3> m_sendQueue;
    Priority m_sentMessagePriority;
    bool m_waitingForEcho;
    boost::asio::steady_timer m_waitingForEchoTimer;
    bool m_waitingForResponse;
    boost::asio::steady_timer m_waitingForResponseTimer;

    TriState m_globalPower;
    std::function<void(bool)> m_onGlobalPowerChanged;

    TriState m_emergencyStop;
    std::function<void()> m_onIdle;

    std::shared_ptr<Clock> m_clock;
    boost::signals2::connection m_clockChangeConnection;
    std::atomic<FastClock> m_fastClock;

    boost::asio::steady_timer m_fastClockSyncTimer;
    bool m_fastClockSupported = true;

    bool m_lncvActive = false;
    uint16_t m_lncvModuleId = 0;
    uint16_t m_lncvModuleAddress = 0;
    OnLNCVReadResponse m_onLNCVReadResponse;

    DecoderController* m_decoderController;
    std::unordered_map<uint16_t, uint8_t> m_addressToSlot;
    std::unordered_map<uint8_t, LocoSlot> m_slots;
    std::unordered_map<uint16_t, std::vector<std::byte>> m_pendingSlotMessages;

    InputController* m_inputController;
    std::array<TriState, 4096> m_inputValues;

    OutputController* m_outputController;
    std::array<OutputPairValue, accessoryOutputAddressMax - accessoryOutputAddressMin + 1> m_outputValues;

    IdentificationController* m_identificationController;

    const std::filesystem::path m_debugDir;
    std::unique_ptr<PCAP> m_pcap;

    Config m_config;

    Kernel(std::string logId_, const Config& config, bool simulation);

    LocoSlot* getLocoSlot(uint8_t slot, bool sendSlotDataRequestIfNew = true);
    LocoSlot* getLocoSlotByAddress(uint16_t address);
    void clearLocoSlot(uint8_t slot);

    std::shared_ptr<Decoder> getDecoder(uint16_t address);

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    inline const Message& lastSentMessage() const
    {
      return m_sendQueue[m_sentMessagePriority].front();
    }

    void resume();

    void send(const Message& message, Priority priority = NormalPriority);
    template<class T>
    void postSend(const T& message)
    {
      assert(sizeof(message) == message.size());
      m_ioContext.post(
        [this, message]()
        {
          send(message);
        });
    }
    template<class T>
    void postSend(const T& message, Priority priority)
    {
      assert(sizeof(message) == message.size());
      m_ioContext.post(
        [this, message, priority]()
        {
          send(message, priority);
        });
    }
    void send(uint16_t address, Message& message, uint8_t& slot);
    template<class T>
    inline void send(uint16_t address, T& message)
    {
      send(address, message, message.slot);
    }
    template<class T>
    void postSend(uint16_t address, const T& message)
    {
      m_ioContext.post(
        [this, address, message]()
        {
          T msg(message);
          send(address, msg, msg.slot);
        });
    }
    void sendNextMessage();

    void waitingForEchoTimerExpired(const boost::system::error_code& ec);
    void waitingForResponseTimerExpired(const boost::system::error_code& ec);

    void setFastClockMaster(bool enable);
    void disableClockEvents();
    void enableClockEvents();

    void startFastClockSyncTimer();
    void stopFastClockSyncTimer();
    void fastClockSyncTimerExpired(const boost::system::error_code& ec);

    template<uint8_t First, uint8_t Last, class T>
    bool updateFunctions(LocoSlot& slot, const T& message);

    void startPCAP(PCAPOutput pcapOutput);

  public:
    static constexpr uint16_t inputAddressMin = 1;
    static constexpr uint16_t inputAddressMax = 4096;
    static constexpr uint16_t identificationAddressMin = 1;
    static constexpr uint16_t identificationAddressMax = 4096;

    Kernel(const Kernel&) = delete;
    Kernel& operator =(const Kernel&) = delete;
    ~Kernel();

#ifndef NDEBUG
    bool isKernelThread() const
    {
      return std::this_thread::get_id() == m_thread.get_id();
    }
#endif

    /**
     * @brief Create kernel and IO handler
     *
     * @param[in] config LocoNet configuration
     * @param[in] args IO handler arguments
     * @return The kernel instance
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
    void setOnGlobalPowerChanged(std::function<void(bool)> callback);

    /**
     * @brief ...
     *
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    void setOnIdle(std::function<void()> callback);

    /**
     * @brief Set clock for LocoNet fast clock
     *
     * @param[in] clock The clock
     * @note This function may not be called when the kernel is running.
     */
    void setClock(std::shared_ptr<Clock> clock);

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
     * @brief Set the identification controller
     *
     * @param[in] identificationController The identification controller
     * @note This function may not be called when the kernel is running.
     */
    void setIdentificationController(IdentificationController* identificationController);

    /**
     * @brief Start the kernel and IO handler
     */
    void start();

    /**
     * @brief Stop the kernel and IO handler
     */
    void stop();

    /**
     * \brief ...
     *
     * This must be called by the IO handler when the connection is ready.
     *
     * \note This function must run in the kernel's IO context
     */
    void started() final;

    /**
     * @brief ...
     *
     * This must be called by the IO handler whenever a LocoNet message is received.
     *
     * @param[in] message The received LocoNet message
     * @note This function must run in the kernel's IO context
     */
    void receive(const Message& message);

    void setState(bool powerOn, bool run);

    //! \brief Send LocoNet packet
    //! \param[in] packet LocoNet packet bytes, exluding checksum.
    //! \return \c true if send, \c false otherwise.
    bool send(std::span<uint8_t> packet);

    //! \brief Send immediate DCC packet
    //! \param[in] dccPacket DCC packet byte, exluding checksum. Length is limited to 5.
    //! \param[in] repeat DCC packet repeat count 0..7
    //! \return \c true if send to command station, \c false otherwise.
    bool immPacket(std::span<const uint8_t> dccPacket, uint8_t repeat);

    template<class T, std::enable_if_t<std::is_trivially_copyable_v<T> && sizeof(T) <= 5, bool> = true>
    bool immPacket(const T& dccPacket, uint8_t repeat)
    {
      return immPacket(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&dccPacket), sizeof(T)), repeat);
    }

    /**
     *
     *
     */
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);

    /**
     *
     * \param[in] channel Output channel
     * @param[in] address Output address
     * @param[in] value Output value
     * @return \c true if send successful, \c false otherwise.
     */
    bool setOutput(OutputChannel channel, uint16_t address, OutputValue value);

    /**
     * \brief Simulate input change
     * \param[in] address Input address, 1..4096
     * \param[in] action Simulation action to perform
     */
    void simulateInputChange(uint16_t address, SimulateInputAction action);

    void lncvStart(uint16_t moduleId, uint16_t moduleAddress);
    void lncvRead(uint16_t lncv);
    void lncvWrite(uint16_t lncv, uint16_t value);
    void lncvStop();
    void setOnLNCVReadResponse(OnLNCVReadResponse callback);
};

}

#endif

