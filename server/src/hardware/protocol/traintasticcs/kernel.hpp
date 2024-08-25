/**
 * server/src/hardware/protocol/loconet/kernel.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICCS_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICCS_KERNEL_HPP

#include "../kernelbase.hpp"
#include <map>
#include <list>
#include <boost/asio/steady_timer.hpp>
#include "config.hpp"
#include "iohandler/iohandler.hpp"

class InputController;
class ThrottleController;
class HardwareThrottle;
enum class DecoderProtocol : uint8_t;
enum class SimulateInputAction;

namespace TraintasticCS {

struct Message;
enum class Command : uint8_t;
enum class Board : uint8_t;
enum class ThrottleChannel : uint8_t;

class Kernel final : public ::KernelBase
{
  public:
    static constexpr uint32_t s88AddressMin = 1;
    static constexpr uint32_t s88AddressMax = 8 * Config::S88::moduleCountMax;

    struct InputChannel
    {
      //static constexpr uint32_t loconet = 1;
      //static constexpr uint32_t xpressnet = 2;
      static constexpr uint32_t s88 = 3;
    };

    inline static const std::vector<uint32_t> inputChannels = {
      //InputChannel::loconet,
      //InputChannel::xpressnet,
      InputChannel::s88,
    };

    inline static const std::vector<std::string_view> inputChannelNames = {
      //"$hardware:loconet$",
      //"$hardware:xpressnet$",
      "$hardware:s88$",
    };

  private:
    //! Startup states, executed in order.
    enum class State
    {
      Initial, // must be first
      Reset,
      GetInfo,
      InitXpressNet,
      InitS88,
      Started // must be last
    };

    struct ThrottleInfo
    {
      DecoderProtocol protocol;
      uint16_t address;
      std::shared_ptr<HardwareThrottle> throttle;
    };
    using Throttles = std::map<std::pair<ThrottleChannel, uint16_t>, ThrottleInfo>;

    std::unique_ptr<IOHandler> m_ioHandler;
    const bool m_simulation;
    State m_state = State::Initial;
    struct
    {
      Board board;
      struct
      {
        uint8_t major;
        uint8_t minor;
        uint8_t patch;
      } version;
    } m_info;
    boost::asio::steady_timer m_responseTimeout;
    std::list<std::pair<Command, std::chrono::steady_clock::time_point>> m_waitingForResponse;
    boost::asio::steady_timer m_pingTimeout;

    InputController* m_inputController = nullptr;

    ThrottleController* m_throttleController = nullptr;
    Throttles m_throttles;

    Config m_config;

    Kernel(std::string logId_, const Config& config, bool simulation);

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    void send(const Message& message);

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

    void startResponseTimeout(Command command);
    void restartResponseTimeoutTimer();
    void restartPingTimeout();

    const std::shared_ptr<HardwareThrottle>& getThrottle(ThrottleChannel channel, uint16_t throttleId, DecoderProtocol protocol, uint16_t address, bool steal = false);

    void changeState(State value);
    inline void nextState()
    {
      assert(m_state != State::Started);
      changeState(static_cast<State>(static_cast<std::underlying_type_t<State>>(m_state) + 1));
    }

  public:
    Kernel(const Kernel&) = delete;
    Kernel& operator =(const Kernel&) = delete;
    ~Kernel() final;

#ifndef NDEBUG
    bool isKernelThread() const
    {
      return std::this_thread::get_id() == m_thread.get_id();
    }
#endif

    /**
     * \brief Create kernel and IO handler
     *
     * \param[in] config Traintastic CS configuration
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
     * \brief Set Traintastic CS configuration
     *
     * \param[in] config The Traintastic CS configuration
     */
    void setConfig(const Config& config);

    /**
     * \brief Set the input controller
     *
     * \param[in] inputController The input controller
     * \note This function may not be called when the kernel is running.
     */
    inline void setInputController(InputController* inputController)
    {
      //assert(!m_running);
      m_inputController = inputController;
    }

    /**
     * \brief Set the throttle controller
     *
     * \param[in] throttleController The throttle controller
     * \note This function may not be called when the kernel is running.
     */
    inline void setThrottleController(ThrottleController* throttleController)
    {
      //assert(!m_running);
      m_throttleController = throttleController;
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
     * This must be called by the IO handler when the connection is ready.
     *
     * \note This function must run in the kernel's IO context
     */
    void started() final;

    /**
     * \brief ...
     *
     * This must be called by the IO handler whenever a LocoNet message is received.
     *
     * \param[in] message The received LocoNet message
     * \note This function must run in the kernel's IO context
     */
    void receive(const Message& message);

    void inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action);
};

}

#endif
