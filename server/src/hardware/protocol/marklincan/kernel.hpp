/**
 * server/src/hardware/protocol/marklincan/kernel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_KERNEL_HPP

#include <memory>
#include <array>
#include <thread>
#include <filesystem>
#include <boost/asio/io_context.hpp>
#include <traintastic/enum/tristate.hpp>
#include "config.hpp"
#include "iohandler/iohandler.hpp"
#include "configdatastreamcollector.hpp"

class Decoder;
enum class DecoderChangeFlags;
class DecoderController;
class InputController;
class OutputController;

namespace MarklinCAN {

struct Message;
class LocomotiveList;

class Kernel
{
  public:
    static constexpr uint16_t s88AddressMin = 1;
    static constexpr uint16_t s88AddressMax = 16384;

    static constexpr uint16_t outputMotorolaAddressMin = 1;
    static constexpr uint16_t outputMotorolaAddressMax = 1024 * 2;
    static constexpr uint16_t outputDCCAddressMin = 1;
    static constexpr uint16_t outputDCCAddressMax = 2048 * 2;
    static constexpr uint16_t outputSX1AddressMin = 1;
    static constexpr uint16_t outputSX1AddressMax = 1024 * 2;

    struct OutputChannel
    {
      static constexpr uint32_t motorola = 1;
      static constexpr uint32_t dcc = 2;
      static constexpr uint32_t sx1 = 3;
    };

    inline static const std::vector<uint32_t> outputChannels = {
      OutputChannel::motorola,
      OutputChannel::dcc,
      OutputChannel::sx1,
    };

    inline static const std::vector<std::string_view> outputChannelNames = {
      "$hardware:motorola$",
      "$hardware:dcc$",
      "SX1",
    };

  private:
    boost::asio::io_context m_ioContext;
    std::unique_ptr<IOHandler> m_ioHandler;
    const bool m_simulation;
    std::thread m_thread;
    std::string m_logId;
    std::function<void()> m_onStarted;
    std::function<void()> m_onError;
    std::function<void(const std::shared_ptr<LocomotiveList>&)> m_onLocomotiveListChanged;

    DecoderController* m_decoderController = nullptr;

    InputController* m_inputController = nullptr;
    std::array<TriState, s88AddressMax - s88AddressMin + 1> m_inputValues;

    OutputController* m_outputController = nullptr;
    std::array<TriState, outputMotorolaAddressMax - outputMotorolaAddressMin + 1> m_outputValuesMotorola;
    std::array<TriState, outputDCCAddressMax - outputDCCAddressMin + 1> m_outputValuesDCC;
    std::array<TriState, outputSX1AddressMax - outputSX1AddressMin + 1> m_outputValuesSX1;

    std::unique_ptr<ConfigDataStreamCollector> m_configDataStreamCollector;

    const std::filesystem::path m_debugDir;

    Config m_config;
#ifndef NDEBUG
    bool m_started;
#endif

    Kernel(const Config& config, bool simulation);

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    void send(const Message& message);
    void postSend(const Message& message);

    void receiveConfigData(std::unique_ptr<ConfigDataStreamCollector> configData);

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
     * \brief IO context for kernel and IO handler
     *
     * \return The IO context
     */
    boost::asio::io_context& ioContext() { return m_ioContext; }

    /**
     * \brief Create kernel and IO handler
     *
     * \param[in] config Configuration
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
     * \brief Get object id used for log messages
     * \return The object id
     */
    inline const std::string& logId()
    {
      return m_logId;
    }

    /**
     * \brief Set object id used for log messages
     *
     * \param[in] value The object id
     */
    void setLogId(std::string value) { m_logId = std::move(value); }

    /**
     * \brief Set configuration
     *
     * \param[in] config The configuration
     */
    void setConfig(const Config& config);

    /**
     * \brief ...
     *
     * \param[in] callback ...
     * \note This function may not be called when the kernel is running.
     */
    void setOnStarted(std::function<void()> callback);

    /**
     * \brief Register error handler
     *
     * Once this handler is called the LocoNet communication it stopped.
     *
     * \param[in] callback Handler to call in case of an error.
     * \note This function may not be called when the kernel is running.
     */
    void setOnError(std::function<void()> callback);

    /**
     *
     */
    void setOnLocomotiveListChanged(std::function<void(const std::shared_ptr<LocomotiveList>&)> callback);

    /**
     * \brief Set the decoder controller
     * \param[in] decoderController The decoder controller
     * \note This function may not be called when the kernel is running.
     */
    void setDecoderController(DecoderController* decoderController);

    /**
     * \brief Set the input controller
     *
     * \param[in] inputController The input controller
     * \note This function may not be called when the kernel is running.
     */
    void setInputController(InputController* inputController);

    /**
     * \brief Set the output controller
     *
     * \param[in] outputController The output controller
     * \note This function may not be called when the kernel is running.
     */
    void setOutputController(OutputController* outputController);

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
     * This must be called by the IO handler whenever a Marklin CAN message is received.
     *
     * \param[in] message The received Marklin CAN message
     * \note This function must run in the kernel's IO context
     */
    void receive(const Message& message);

    //! Must be called by the IO handler in case of a fatal error.
    //! This will put the interface in error state
    //! \note This function must run in the event loop thread
    void error();

    void systemStop();
    void systemGo();
    void systemHalt();

    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);

    /**
     * \brief ...
     * \param[in] channel Channel
     * \param[in] address Output address
     * \param[in] value Output value: \c true is on, \c false is off.
     * \return \c true if send successful, \c false otherwise.
     */
    bool setOutput(uint32_t channel, uint16_t address, bool value);
};

}

#endif
