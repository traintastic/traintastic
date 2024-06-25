/**
 * server/src/hardware/protocol/marklincan/kernel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023-2024 Reinder Feenstra
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

#include "../kernelbase.hpp"
#include <memory>
#include <array>
#include <map>
#include <unordered_map>
#include <filesystem>
#include <queue>
#include <boost/asio/steady_timer.hpp>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/enum/outputchannel.hpp>
#include "config.hpp"
#include "node.hpp"
#include "iohandler/iohandler.hpp"
#include "configdatastreamcollector.hpp"
#include "../dcc/dcc.hpp"
#include "../motorola/motorola.hpp"
#include "../../output/outputvalue.hpp"

class Decoder;
enum class DecoderChangeFlags;
class DecoderController;
class InputController;
class OutputController;

namespace MarklinCAN {

struct Message;
class LocomotiveList;

class Kernel : public ::KernelBase
{
  public:
    static constexpr std::string_view nodeDeviceName = "Traintastic";
    static constexpr std::string_view nodeArticleNumber = "1";

    static constexpr uint16_t s88AddressMin = 1;
    static constexpr uint16_t s88AddressMax = 16384;

  private:
    //! Startup states, executed in order.
    enum class State
    {
      Initial, // must be first
      DiscoverNodes,
      SetAccessorySwitchTime,
      DownloadLokList,
      Started // must be last
    };

    struct StatusDataConfigRequest
    {
      uint32_t uid;
      uint8_t index;

      StatusDataConfigRequest(uint32_t uid_, uint8_t index_)
        : uid{uid_}
        , index{index_}
      {
      }
    };

    static constexpr int statusDataConfigRequestRetryCount = 3;

    std::unique_ptr<IOHandler> m_ioHandler;
    const bool m_simulation;
    State m_state = State::Initial;
    std::function<void(const Node& node)> m_onNodeChanged;
    std::queue<StatusDataConfigRequest> m_statusDataConfigRequestQueue; //<! UID+index to request config data from
    int m_statusDataConfigRequestRetries = statusDataConfigRequestRetryCount;
    boost::asio::steady_timer m_statusDataConfigRequestTimer;

    std::function<void(const std::shared_ptr<LocomotiveList>&)> m_onLocomotiveListChanged;

    std::unordered_map<uint32_t, Node> m_nodes;

    DecoderController* m_decoderController = nullptr;
    std::map<uint32_t, uint16_t> m_mfxUIDtoSID;

    InputController* m_inputController = nullptr;
    std::array<TriState, s88AddressMax - s88AddressMin + 1> m_inputValues;

    OutputController* m_outputController = nullptr;
    std::array<OutputPairValue, Motorola::Accessory::addressMax - Motorola::Accessory::addressMin + 1> m_outputValuesMotorola;
    std::array<OutputPairValue, DCC::Accessory::addressMax - DCC::Accessory::addressMin + 1> m_outputValuesDCC;

    std::vector<std::byte> m_statusConfigData;
    std::unique_ptr<ConfigDataStreamCollector> m_configDataStreamCollector;

    const std::filesystem::path m_debugDir;

    Config m_config;

    Kernel(std::string logId_, const Config& config, bool simulation);

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    void send(const Message& message);
    void postSend(const Message& message);

    void receiveStatusDataConfig(uint32_t nodeUID, uint8_t index, const std::vector<std::byte>& statusConfigData);
    void receiveConfigData(std::unique_ptr<ConfigDataStreamCollector> configData);

    void restartStatusDataConfigTimer();

    void nodeChanged(const Node& node);

    void changeState(State value);
    inline void nextState()
    {
      assert(m_state != State::Started);
      changeState(static_cast<State>(static_cast<std::underlying_type_t<State>>(m_state) + 1));
    }

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
     * \brief Create kernel and IO handler
     *
     * \param[in] config Configuration
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
     * \brief Set configuration
     *
     * \param[in] config The configuration
     */
    void setConfig(const Config& config);

    /**
     *
     */
    void setOnLocomotiveListChanged(std::function<void(const std::shared_ptr<LocomotiveList>&)> callback);

    /**
     *
     */
    void setOnNodeChanged(std::function<void(const Node& node)> callback);

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
     * \brief Notify kernel the IO handler is started.
     * \note This function must run in the kernel's IO context
     */
    void started() final;

    /**
     * \brief ...
     *
     * This must be called by the IO handler whenever a Marklin CAN message is received.
     *
     * \param[in] message The received Marklin CAN message
     * \note This function must run in the kernel's IO context
     */
    void receive(const Message& message);

    void systemStop();
    void systemGo();
    void systemHalt();

    void getLocomotiveList();

    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);

    /**
     * \brief ...
     * \param[in] channel Channel
     * \param[in] address Output address
     * \param[in] value Output value
     * \return \c true if send successful, \c false otherwise.
     */
    bool setOutput(OutputChannel channel, uint16_t address, OutputPairValue value);
};

}

#endif
