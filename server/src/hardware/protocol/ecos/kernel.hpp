/**
 * server/src/hardware/protocol/ecos/kernel.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_KERNEL_HPP

#include "../kernelbase.hpp"
#include <unordered_map>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/enum/decoderprotocol.hpp>
#include <traintastic/enum/outputchannel.hpp>
#include "config.hpp"
#include "iohandler/iohandler.hpp"
#include "object/object.hpp"
#include "object/switchprotocol.hpp"
#include "../../output/outputvalue.hpp"

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

class Kernel : public ::KernelBase
{
  friend class Object;
  friend class ECoS;

  public:
    using OnObjectChanged = std::function<void(std::size_t, uint16_t, const std::string&)>;
    using OnObjectRemoved = std::function<void(uint16_t)>;

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

    std::unique_ptr<IOHandler> m_ioHandler;
    const bool m_simulation;

    Objects m_objects;

    std::function<void()> m_onGo;
    std::function<void()> m_onEmergencyStop;
    OnObjectChanged m_onObjectChanged;
    OnObjectRemoved m_onObjectRemoved;

    DecoderController* m_decoderController;
    InputController* m_inputController;
    OutputController* m_outputController;

    Config m_config;

    Kernel(std::string logId_, const Config& config, bool simulation);

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    bool objectExists(uint16_t objectId) const;
    void addObject(std::unique_ptr<Object> object);
    void objectChanged(Object& object);
    void removeObject(uint16_t objectId);

    //const std::unique_ptr<ECoS&> ecos();
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

#ifndef NDEBUG
    bool isKernelThread() const
    {
      return std::this_thread::get_id() == m_thread.get_id();
    }
#endif

    /**
     * @brief Create kernel and IO handler
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
     * @brief Set ECoS configuration
     * @param[in] config The LocoNet configuration
     */
    void setConfig(const Config& config);

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

    void setOnObjectChanged(OnObjectChanged callback);
    void setOnObjectRemoved(OnObjectRemoved callback);

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
     * \brief Notify kernel the IO handler is started.
     * \note This function must run in the kernel's IO context
     */
    void started() final;

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
     * @param[in] id Output id/address
     * @param[in] value Output value
     * @return \c true if send successful, \c false otherwise.
     */
    bool setOutput(OutputChannel channel, uint32_t id, OutputValue value);

    void simulateInputChange(uint32_t channel, uint32_t address, SimulateInputAction action);

    void switchManagerSwitched(SwitchProtocol protocol, uint16_t address, OutputPairValue value);
    void switchStateChanged(uint16_t objectId, uint8_t state);

    void feedbackStateChanged(Feedback& object, uint8_t port, TriState value);
};

}

#endif
