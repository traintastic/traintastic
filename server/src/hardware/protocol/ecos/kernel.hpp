/**
 * server/src/hardware/protocol/ecos/kernel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_KERNEL_HPP

#include <thread>
#include <unordered_map>
#include <boost/asio/io_context.hpp>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/enum/decoderprotocol.hpp>
#include "config.hpp"
#include "iohandler/iohandler.hpp"
#include "object/object.hpp"
#include "object/switchprotocol.hpp"

class Decoder;
enum class DecoderChangeFlags;
class DecoderController;
enum class SimulateInputAction;
class InputController;
class OutputController;

namespace ECoS {

class ECoS;
class Locomotive;
class SwitchManager;
class Feedback;
struct Simulation;

class Kernel
{
  friend class Object;
  friend class ECoS;

  public:
    static constexpr uint16_t s88AddressMin = 1;
    static constexpr uint16_t s88AddressMax = 1000; //!< \todo what is the maximum
    static constexpr uint16_t ecosDetectorAddressMin = 1;
    static constexpr uint16_t ecosDetectorAddressMax = 1000; //!< \todo what is the maximum

    struct InputChannel
    {
      static constexpr uint32_t s88 = 1;
      static constexpr uint32_t ecosDetector = 2;
    };

    inline static const std::vector<uint32_t> inputChannels = {
      InputChannel::s88,
      InputChannel::ecosDetector,
    };

    inline static const std::vector<std::string_view> inputChannelNames = {
      "$hardware:s88$",
      "$ecos_channel:ecos_detector$",
    };

    static constexpr uint16_t outputDCCAddressMin = 1;
    static constexpr uint16_t outputDCCAddressMax = 1000; //!< \todo what is the maximum
    static constexpr uint16_t outputMotorolaAddressMin = 1;
    static constexpr uint16_t outputMotorolaAddressMax = 1000; //!< \todo what is the maximum

    struct OutputChannel
    {
      static constexpr uint32_t dcc = 1;
      static constexpr uint32_t motorola = 2;
    };

    inline static const std::vector<uint32_t> outputChannels = {
      OutputChannel::dcc,
      OutputChannel::motorola,
    };

    inline static const std::vector<std::string_view> outputChannelNames = {
      "$hardware:dcc$",
      "$hardware:motorola$",
    };

  private:
    class Objects : public std::unordered_map<uint16_t, std::unique_ptr<Object>>
    {
      public:
        template<class T>
        inline void add(std::unique_ptr<T> object)
        {
          const auto id = object->id();
          emplace(id, std::move(object));
        }
    };

    boost::asio::io_context m_ioContext;
    std::unique_ptr<IOHandler> m_ioHandler;
    const bool m_simulation;
    std::thread m_thread;
    std::string m_logId;
    std::function<void()> m_onStarted;

    Objects m_objects;

    std::function<void()> m_onGo;
    std::function<void()> m_onEmergencyStop;

    DecoderController* m_decoderController;
    InputController* m_inputController;
    OutputController* m_outputController;

    Config m_config;
#ifndef NDEBUG
    bool m_started;
#endif

    Kernel(const Config& config, bool simulation);

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    ECoS& ecos();
    void ecosGoChanged(TriState value);

    Locomotive* getLocomotive(DecoderProtocol protocol, uint16_t address, uint8_t speedSteps);

    SwitchManager& switchManager();

  public:// REMOVE!! just for testing
    void postSend(const std::string& message)
    {
      m_ioContext.post(
        [this, message]()
        {
          send(message);
        });
    }

    void send(std::string_view message);

  public:
    Kernel(const Kernel&) = delete;
    Kernel& operator =(const Kernel&) = delete;

    /**
     * @brief IO context for ECoS kernel and IO handler
     * @return The IO context
     */
    boost::asio::io_context& ioContext() { return m_ioContext; }

    /**
     * @brief Create kernel and IO handler
     * @param[in] config LocoNet configuration
     * @param[in] args IO handler arguments
     * @return The kernel instance
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
     * @brief Access the IO handler
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
     * @brief Get object id used for log messages
     * @return The object id
     */
    inline const std::string& logId()
    {
      return m_logId;
    }

    /**
     * @brief Set object id used for log messages
     * @param[in] value The object id
     */
    void setLogId(std::string value) { m_logId = std::move(value); }

    /**
     * @brief Set ECoS configuration
     * @param[in] config The LocoNet configuration
     */
    void setConfig(const Config& config);

    /**
     * @brief ...
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    void setOnStarted(std::function<void()> callback);

    /**
     * @brief ...
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    void setOnEmergencyStop(std::function<void()> callback);

    /**
     * @brief ...
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    void setOnGo(std::function<void()> callback);

    /**
     * @brief Set the decoder controller
     * @param[in] decoderController The decoder controller
     * @note This function may not be called when the kernel is running.
     */
    void setDecoderController(DecoderController* decoderController);

    /**
     * @brief Set the input controller
     * @param[in] inputController The input controller
     * @note This function may not be called when the kernel is running.
     */
    void setInputController(InputController* inputController);

    /**
     * @brief Set the output controller
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
     * @param[out] simulation Get simulation data (optional)
     */
    void stop(Simulation* simulation);

    /**
     * @brief ...
     *
     * This must be called by the IO handler whenever a ECoS message is received.
     *
     * @param[in] message The received ECoS message
     * @note This function must run in the kernel's IO context
     */
    void receive(std::string_view message);

    /**
     * @brief ...
     */
    void emergencyStop();

    /**
     * @brief ...
     */
    void go();

    /**
     * @brief ...
     * @param[in] decoder ...
     * @param[in] changes ...
     * @param[in] functionNumber ...
     */
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);

    /**
     * @brief ...
     * @param[in] channel Channel
     * @param[in] address Output address
     * @param[in] value Output value: \c true is on, \c false is off.
     * @return \c true if send successful, \c false otherwise.
     */
    bool setOutput(uint32_t channel, uint16_t address, bool value);

    void simulateInputChange(uint32_t channel, uint32_t address, SimulateInputAction action);

    void switchManagerSwitched(SwitchProtocol protocol, uint16_t address);

    void feedbackStateChanged(Feedback& object, uint8_t port, TriState value);
};

}

#endif
