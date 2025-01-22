/**
 * server/src/hardware/protocol/dccex/kernel.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCCEX_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCCEX_KERNEL_HPP

#include "../kernelbase.hpp"
#include <array>
#include <unordered_map>
#include <boost/asio/steady_timer.hpp>
#include <traintastic/enum/outputchannel.hpp>
#include "config.hpp"
#include "iohandler/iohandler.hpp"
#include "../../output/outputvalue.hpp"

class Decoder;
enum class DecoderChangeFlags;
class DecoderController;
enum class SimulateInputAction;
class InputController;
class OutputController;

namespace DCCEX {

struct Message;

class Kernel : public ::KernelBase
{
  public:
    static constexpr uint32_t idMin = 0;
    static constexpr uint32_t idMax = 32767;

  private:
    std::unique_ptr<IOHandler> m_ioHandler;
    const bool m_simulation;
    boost::asio::steady_timer m_startupDelayTimer;

    TriState m_powerOn;
    TriState m_emergencyStop;
    std::function<void(bool)> m_onPowerOnChanged;

    DecoderController* m_decoderController;

    InputController* m_inputController;
    std::unordered_map<uint16_t, bool> m_inputValues;

    OutputController* m_outputController;

    Config m_config;

    Kernel(std::string logId_, const Config& config, bool simulation);

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    void postSend(const std::string& message)
    {
      m_ioContext.post(
        [this, message]()
        {
          send(message);
        });
    }

    void send(std::string_view message);

    void startupDelayExpired(const boost::system::error_code& ec);

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
     * @brief Create kernel and IO handler
     *
     * @param[in] config DCC-EX configuration
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
     * @brief Set DCC-EX configuration
     *
     * @param[in] config The DCC-EX configuration
     */
    void setConfig(const Config& config);

    /**
     * @brief ...
     *
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    inline void setOnPowerOnChanged(std::function<void(bool)> callback)
    {
      assert(!m_started);
      m_onPowerOnChanged = std::move(callback);
    }

    /**
     * @brief Set the decoder controller
     *
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
    inline void setInputController(InputController* inputController)
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
    inline void setOutputController(OutputController* outputController)
    {
      assert(!m_started);
      m_outputController = outputController;
    }

    /**
     * @brief Start the kernel and IO handler
     */
    void start();

    /**
     * @brief Stop the kernel and IO handler
     */
    void stop();

    /**
     * \brief Notify kernel the IO handler is started.
     * \note This function must run in the kernel's IO context
     */
    void started() final;

    /**
     * @brief ...
     *
     * This must be called by the IO handler whenever a DCC-EX message is received.
     *
     * @param[in] message The received DCC-EX message
     * @note This function must run in the kernel's IO context
     */
    void receive(std::string_view message);

    /**
     *
     *
     */
    void powerOn();

    /**
     *
     *
     */
    void powerOff();

    /**
     *
     *
     */
    void emergencyStop();

    /**
     *
     *
     */
    void clearEmergencyStop();

    /**
     *
     *
     */
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);

    /**
     *
     * @param[in] channel Output channel
     * @param[in] address Output address
     * @param[in] value Output value
     * @return \c true if send successful, \c false otherwise.
     */
    bool setOutput(OutputChannel channel, uint16_t address, OutputValue value);

    /**
     * \brief Simulate input change
     * \param[in] address Input address, #idMin..#idMax
     * \param[in] action Simulation action to perform
     */
    void simulateInputChange(uint16_t address, SimulateInputAction action);
};

}

#endif
