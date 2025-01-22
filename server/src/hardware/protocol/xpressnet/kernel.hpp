/**
 * server/src/hardware/protocol/xpressnet/kernel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_KERNEL_HPP

#include "../kernelbase.hpp"
#include <array>
#include <boost/asio/steady_timer.hpp>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/enum/outputpairvalue.hpp>
#include "config.hpp"
#include "iohandler/iohandler.hpp"

class Decoder;
enum class DecoderChangeFlags;
class DecoderController;
enum class SimulateInputAction;
class InputController;
class OutputController;

namespace XpressNet {

struct Message;

class Kernel : public ::KernelBase
{
  public:
    static constexpr uint16_t inputAddressMin = 1;
    static constexpr uint16_t inputAddressMax = 2048;
    static constexpr uint16_t accessoryOutputAddressMin = 1;
    static constexpr uint16_t accessoryOutputAddressMax = 1024;

  private:
    std::unique_ptr<IOHandler> m_ioHandler;
    const bool m_simulation;

    TriState m_trackPowerOn;
    TriState m_emergencyStop;
    std::function<void()> m_onNormalOperationResumed;
    std::function<void()> m_onTrackPowerOff;
    std::function<void()> m_onEmergencyStop;

    DecoderController* m_decoderController;

    InputController* m_inputController;
    std::array<TriState, inputAddressMax - inputAddressMin + 1> m_inputValues;

    OutputController* m_outputController;
    //std::array<OutputPairValue, accessoryOutputAddressMax - accessoryOutputAddressMin + 1> m_outputValues;

    Config m_config;

    Kernel(std::string logId_, const Config& config, bool simulation);

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
     * @param[in] config XpressNet configuration
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
     * @brief Set XpressNet configuration
     *
     * @param[in] config The XpressNet configuration
     */
    void setConfig(const Config& config);

    /**
     * @brief ...
     *
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    inline void setOnNormalOperationResumed(std::function<void()> callback)
    {
      assert(!m_started);
      m_onNormalOperationResumed = std::move(callback);
    }

    /**
     * @brief ...
     *
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
     *
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    inline void setOnEmergencyStop(std::function<void()> callback)
    {
      assert(!m_started);
      m_onEmergencyStop = std::move(callback);
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
     * This must be called by the IO handler whenever a XpressNet message is received.
     *
     * @param[in] message The received XpressNet message
     * @note This function must run in the kernel's IO context
     */
    void receive(const Message& message);

    /**
     *
     *
     */
    void resumeOperations();

    /**
     *
     *
     */
    void stopOperations();

    /**
     *
     *
     */
    void stopAllLocomotives();

    /**
     *
     *
     */
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);

    /**
     *
     * @param[in] address Output address, #accessoryOutputAddressMin..#accessoryOutputAddressMax
     * @param[in] value Output value: \c First or \c Second .
     * @return \c true if send successful, \c false otherwise.
     */
    bool setOutput(uint16_t address, OutputPairValue value);

    /**
     * \brief Simulate input change
     * \param[in] address Input address, #inputAddressMin..#inputAddressMax
     * \param[in] action Simulation action to perform
     */
    void simulateInputChange(uint16_t address, SimulateInputAction action);
};

}

#endif
