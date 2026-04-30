/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSKERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSKERNEL_HPP

#include "../kernelbase.hpp"
#include <map>
#include <span>
#include <set>
#include <queue>
#include <optional>
#include <boost/asio/steady_timer.hpp>
#include <traintastic/enum/direction.hpp>
#include "cbusconfig.hpp"
#include "iohandler/cbusiohandler.hpp"
#include "messages/cbusenginemessages.hpp"
#include "messages/cbusnodeparametermessages.hpp"
#include "../dcc/messages.hpp"

namespace CBUS {

class IOHub;

class Kernel : public ::KernelBase
{
public:
  std::function<void(uint8_t canId, uint16_t nodeNumber, uint8_t manufacturerId, uint8_t moduleId, bool flimMode, bool supportsServiceDiscovery)> onPresenceOfNode;
  std::function<void(uint8_t canId, uint16_t nodeNumber, NodeParameter parameter, uint8_t value)> onNodeParameterResponse;
  std::function<void()> onTrackOff;
  std::function<void()> onTrackOn;
  std::function<void()> onEmergencyStop;
  std::function<void(uint8_t session, bool external, uint16_t address, bool isLongAddress)> onEngineSessionAcquire;
  std::function<void(uint8_t session, uint8_t speed, bool forward)> onEngineSpeedDirectionChanged;
  std::function<void(uint8_t session, uint8_t number, bool on)> onEngineFunctionChanged;
  std::function<void(uint8_t session)> onEngineSessionReleased;
  std::function<void(uint16_t address, bool isLongAddress)> onEngineSessionCancelled;
  std::function<void(uint16_t eventNumber, bool on)> onShortEvent;
  std::function<void(uint16_t nodeNumber, uint16_t eventNumber, bool on)> onLongEvent;

  /**
   * @brief Create kernel and IO handler
   *
   * @param[in] config CBUS configuration
   * @param[in] args IO handler arguments
   * @return The kernel instance
   */
  template<class IOHandlerType, class... Args>
  static std::unique_ptr<Kernel> create(std::string logId_, const Config& config, uint8_t canId, Args... args)
  {
    static_assert(std::is_base_of_v<IOHandler, IOHandlerType>);
    std::unique_ptr<Kernel> kernel{new Kernel(std::move(logId_), config, canId, isSimulation<IOHandlerType>())};
    kernel->setIOHandler(std::make_unique<IOHandlerType>(*kernel, std::forward<Args>(args)...));
    return kernel;
  }

#ifndef NDEBUG
  bool isKernelThread() const
  {
    return std::this_thread::get_id() == m_thread.get_id();
  }
#endif

  /**
   * @brief Set CBUS configuration
   *
   * @param[in] config The CBUS configuration
   */
  void setConfig(const Config& config);

  void setRequestEventsDuringInitialize(std::vector<uint16_t> shortEvents, std::vector<std::pair<uint16_t,uint16_t>> longEvents);

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

  size_t registerOnReceive(OpCode opCode, std::function<void(uint8_t, const Message&)> callback);
  void unregisterOnReceive(size_t handle);

  void trackOff();
  void trackOn();
  void requestEmergencyStop();

  void setEngineSpeedDirection(uint16_t address, bool longAddress, uint8_t speedStep, uint8_t speedSteps, bool eStop, bool directionForward);
  void setEngineFunction(uint16_t address, bool longAddress, uint8_t number, bool value);

  void setAccessoryShort(uint16_t deviceNumber, bool on);
  void setAccessory(uint16_t nodeNumber, uint16_t eventNumber, bool on);

  void setDccAccessory(uint16_t address, bool secondOutput);
  void setDccAdvancedAccessoryValue(uint16_t address, uint8_t aspect);

  bool send(std::vector<uint8_t> message);
  bool sendDCC(std::vector<uint8_t> dccPacket, uint8_t repeat);

private:
  //! Startup states, executed in order.
  enum class State
  {
    Initial, // must be first
    QueryNodes,
    ReadNodeParameters,
    GetCommandStationStatus,
    RequestShortEvents,
    RequestLongEvents,
    Started // must be last
  };

  enum class Owner
  {
    Traintastic = 1,
    CBUS = 2,
  };

  struct Engine
  {
    std::optional<uint8_t> session;
    Owner owner;
    uint8_t speed = 0;
    uint8_t speedSteps = 126;
    bool directionForward = true;
    std::map<uint8_t, bool> functions;
    std::chrono::steady_clock::time_point lastCommand;
  };

  std::unique_ptr<IOHandler> m_ioHandler;
  std::shared_ptr<IOHub> m_hub;
  const uint8_t m_canId;
  const bool m_simulation;
  State m_state = State::Initial;
  boost::asio::steady_timer m_initializationTimer;
  std::queue<ReadNodeParameter> m_readNodeParameters;
  std::vector<uint16_t> m_initializationRequestShortEvents;
  std::vector<std::pair<uint16_t,uint16_t>> m_initializationRequestLongEvents;
  Config m_config;

  size_t m_onReceiveHandle = 0; // owned by eventloop thread
  std::unordered_map<size_t, std::function<void(uint8_t, const Message&)>> m_onReceiveCallbacks; // owned by eventloop thread
  std::unordered_map<size_t, OpCode> m_onReceiveFilters; // owned by kernel thread

  bool m_trackOn = false;
  uint8_t m_engineKeepAliveSession;
  bool m_engineKeepAliveTimerActive = false;
  boost::asio::steady_timer m_engineKeepAliveTimer;
  std::map<uint16_t, Engine> m_engines;
  std::map<uint16_t, Owner> m_engineGLOCs;
  std::queue<std::pair<std::chrono::steady_clock::time_point, DCC::SetSimpleAccessory>> m_dccAccessoryQueue;
  boost::asio::steady_timer m_dccAccessoryTimer;

  Kernel(std::string logId_, const Config& config, uint8_t canId, bool simulation);

  Kernel(const Kernel&) = delete;
  Kernel& operator =(const Kernel&) = delete;

  void setIOHandler(std::unique_ptr<IOHandler> handler);

  void send(const Message& message);
  void sendGetEngineSession(uint16_t address, bool longAddress);
  void sendSetEngineSessionMode(uint8_t session, uint8_t speedSteps);
  void sendSetEngineSpeedDirection(uint8_t session, uint8_t speed, bool directionForward);
  void sendSetEngineFunction(uint8_t session, uint8_t number, bool value);

  void receive(const CAN::Message& canMessage);
  void receiveGLOC(uint16_t address, bool longAddress, GetEngineSession::Mode mode);
  void receiveDFUN(const SetEngineFunctions& message);
  void receiveDFNOx(const SetEngineFunction& message);
  void receiveDSPD(const SetEngineSpeedDirection& message);
  void receiveKLOC(const ReleaseEngine& message);
  void receiveShortEvent(uint16_t eventNumber, bool on);
  void receiveLongEvent(uint16_t nodeNumber, uint16_t eventNumber, bool on);

  inline void nextState()
  {
    assert(m_state != State::Started);
    changeState(static_cast<State>(static_cast<std::underlying_type_t<State>>(m_state) + 1));
  }

  void changeState(State value);

  void readNodeParameter();
  void requestShortEvent();
  void requestLongEvent();

  void restartInitializationTimer(std::chrono::milliseconds timeout);
  void restartEngineKeepAliveTimer();
  void startDccAccessoryTimer();
};

}

#endif
