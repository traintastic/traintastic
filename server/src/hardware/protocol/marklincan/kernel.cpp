/**
 * server/src/hardware/protocol/marklincan/kernel.cpp
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

#include "kernel.hpp"
#include <nlohmann/json.hpp>
#include <version.hpp>
#include "messages.hpp"
#include "message/configdata.hpp"
#include "message/statusdataconfig.hpp"
#include "locomotivelist.hpp"
#include "uid.hpp"
#include "../dcc/dcc.hpp"
#include "../motorola/motorola.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../decoder/decodercontroller.hpp"
#include "../../input/inputcontroller.hpp"
#include "../../output/outputcontroller.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../traintastic/traintastic.hpp"
#include "../../../utils/inrange.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../utils/tohex.hpp"
#include "../../../utils/writefile.hpp"
#include "../../../utils/zlib.hpp"

namespace MarklinCAN {

static std::tuple<bool, DecoderProtocol, uint16_t> uidToProtocolAddress(uint32_t uid)
{
  if(inRange(uid, UID::Range::locomotiveMotorola))
    return {true, DecoderProtocol::Motorola, uid - UID::Range::locomotiveMotorola.first};
  if(inRange(uid, UID::Range::locomotiveMFX))
    return {true, DecoderProtocol::MFX, uid - UID::Range::locomotiveMFX.first};
  if(inRange(uid, UID::Range::locomotiveDCC))
  {
    //! \todo Handle long address < 128
    const uint16_t address = uid - UID::Range::locomotiveDCC.first;
    if(address <= DCC::addressShortMax)
      return {true, DecoderProtocol::DCCShort, address};

    return {true, DecoderProtocol::DCCLong, address};
  }
  return {false, DecoderProtocol::None, 0};
}

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_statusDataConfigRequestTimer{m_ioContext}
  , m_debugDir{Traintastic::instance->debugDir()}
  , m_config{config}
{
  assert(isEventLoopThread());
  (void)m_simulation;
}

void Kernel::setConfig(const Config& config)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, newConfig=config]()
    {
      if(m_config.defaultSwitchTime != newConfig.defaultSwitchTime)
        send(AccessorySwitchTime(newConfig.defaultSwitchTime / 10));

      m_config = newConfig;
    });
}

void Kernel::setOnLocomotiveListChanged(std::function<void(const std::shared_ptr<LocomotiveList>&)> callback)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_onLocomotiveListChanged = std::move(callback);
}

void Kernel::setOnNodeChanged(std::function<void(const Node& node)> callback)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_onNodeChanged = std::move(callback);
}

void Kernel::setDecoderController(DecoderController* decoderController)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_decoderController = decoderController;
}

void Kernel::setInputController(InputController* inputController)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_inputController = inputController;
}

void Kernel::setOutputController(OutputController* outputController)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_outputController = outputController;
}

void Kernel::start()
{
  assert(isEventLoopThread());
  assert(m_ioHandler);
  assert(!m_started);

  // reset all state values
  m_inputValues.fill(TriState::Undefined);
  m_outputValuesMotorola.fill(OutputPairValue::Undefined);
  m_outputValuesDCC.fill(OutputPairValue::Undefined);

  m_thread = std::thread(
    [this]()
    {
      setThreadName("marklin_can");
      auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
      m_ioContext.run();
    });

  m_ioContext.post(
    [this]()
    {
      try
      {
        m_ioHandler->start();
      }
      catch(const LogMessageException& e)
      {
        EventLoop::call(
          [this, e]()
          {
            Log::log(logId, e.message(), e.args());
            error();
          });
      }
    });

#ifndef NDEBUG
  m_started = true;
#endif
}

void Kernel::stop()
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this]()
    {
      m_ioHandler->stop();
    });

  m_ioContext.stop();

  m_thread.join();

#ifndef NDEBUG
  m_started = false;
#endif
}

void Kernel::started()
{
  // add Traintastic to the node list
  {
    Node node;
    node.uid = m_config.nodeUID;
    node.deviceName = nodeDeviceName;
    node.articleNumber = nodeArticleNumber;
    node.serialNumber = m_config.nodeSerialNumber;
    node.softwareVersionMajor = TRAINTASTIC_VERSION_MAJOR;
    node.softwareVersionMinor = TRAINTASTIC_VERSION_MINOR;
    node.deviceId = DeviceId::Traintastic;

    nodeChanged(node);

    m_nodes.emplace(m_config.nodeUID, node);
  }

  nextState();
}

void Kernel::receive(const Message& message)
{
  assert(isKernelThread());

  if(m_config.debugLogRXTX)
    EventLoop::call([this, msg=toString(message)](){ Log::log(logId, LogMessage::D2002_RX_X, msg); });

  switch(message.command())
  {
    case Command::System:
    {
      const auto& system = static_cast<const SystemMessage&>(message);

      switch(system.subCommand())
      {
        case SystemSubCommand::SystemStop:
        case SystemSubCommand::SystemGo:
        case SystemSubCommand::SystemHalt:
          // not (yet) implemented
          break;

        case SystemSubCommand::LocomotiveEmergencyStop:
          if(m_decoderController && system.isResponse())
          {
            auto [success, proto, addr] = uidToProtocolAddress(system.uid());
            if(success)
            {
              EventLoop::call(
                [this, protocol=proto, address=addr]()
                {
                  if(const auto& decoder = m_decoderController->getDecoder(protocol, address))
                    decoder->emergencyStop.setValueInternal(true);
                });
            }
          }
          break;

        case SystemSubCommand::LocomotiveCycleEnd:
          // not (yet) implemented
          break;
        case SystemSubCommand::AccessorySwitchTime:
          if(message.isResponse() && m_state == State::SetAccessorySwitchTime)
            nextState();
          break;

        case SystemSubCommand::Overload:
        case SystemSubCommand::Status:
        case SystemSubCommand::ModelClock:
        case SystemSubCommand::MFXSeek:
          // not (yet) implemented
          break;
      }
      break;
    }
    case Command::Discovery:
    case Command::Bind:
    case Command::Verify:
      // not (yet) implemented
      break;

    case Command::LocomotiveSpeed:
      if(m_decoderController)
      {
        const auto& locomotiveSpeed = static_cast<const LocomotiveSpeed&>(message);
        if(locomotiveSpeed.isResponse() && locomotiveSpeed.hasSpeed())
        {
          auto [success, proto, addr] = uidToProtocolAddress(locomotiveSpeed.uid());
          if(success)
          {
            EventLoop::call(
              [this, protocol=proto, address=addr, throttle=Decoder::speedStepToThrottle(locomotiveSpeed.speed(), LocomotiveSpeed::speedMax)]()
              {
                if(const auto& decoder = m_decoderController->getDecoder(protocol, address))
                {
                  decoder->emergencyStop.setValueInternal(false);
                  decoder->throttle.setValueInternal(throttle);
                }
              });
          }
        }
      }
      break;

    case Command::LocomotiveDirection:
      if(m_decoderController)
      {
        const auto& locomotiveDirection = static_cast<const LocomotiveDirection&>(message);
        if(locomotiveDirection.isResponse() && locomotiveDirection.hasDirection())
        {
          auto [success, proto, addr] = uidToProtocolAddress(locomotiveDirection.uid());
          if(success)
          {
            Direction direction = Direction::Unknown;
            switch(locomotiveDirection.direction())
            {
              case LocomotiveDirection::Direction::Forward:
                direction = Direction::Forward;
                break;

              case LocomotiveDirection::Direction::Reverse:
                direction = Direction::Reverse;
                break;

              case LocomotiveDirection::Direction::Same:
              case LocomotiveDirection::Direction::Inverse:
                break;
            }

            EventLoop::call(
              [this, protocol=proto, address=addr, direction]()
              {
                if(const auto& decoder = m_decoderController->getDecoder(protocol, address))
                  decoder->direction.setValueInternal(direction);
              });
          }
        }
      }
      break;

    case Command::LocomotiveFunction:
      if(m_decoderController)
      {
        const auto& locomotiveFunction = static_cast<const LocomotiveFunction&>(message);
        if(locomotiveFunction.isResponse() && locomotiveFunction.hasValue())
        {
          auto [success, proto, addr] = uidToProtocolAddress(locomotiveFunction.uid());
          if(success)
          {
            EventLoop::call(
              [this, protocol=proto, address=addr, number=locomotiveFunction.number(), value=locomotiveFunction.isOn()]()
              {
                if(const auto& decoder = m_decoderController->getDecoder(protocol, address))
                  decoder->setFunctionValue(number, value);
              });
          }
        }
      }
      break;

    case Command::ReadConfig:
    case Command::WriteConfig:
      // not (yet) implemented
      break;

    case Command::AccessoryControl:
      if(message.isResponse() && (message.dlc == 6 || message.dlc == 8))
      {
        const auto& accessoryControl = static_cast<const AccessoryControl&>(message);
        if(accessoryControl.position() != AccessoryControl::positionOff &&
            accessoryControl.position() != AccessoryControl::positionOn)
          break;

        OutputChannel channel;
        uint32_t address;
        const auto value = accessoryControl.position() == AccessoryControl::positionOff ? OutputPairValue::First : OutputPairValue::Second;

        if(inRange(accessoryControl.uid(), UID::Range::accessoryMotorola))
        {
          channel = OutputChannel::AccessoryMotorola;
          address = 1 + (accessoryControl.uid() - UID::Range::accessoryMotorola.first);
          //if(address > m_outputValuesMotorola.size() || m_outputValuesMotorola[address - 1] == value)
          //  break;
          //m_outputValuesMotorola[address - 1] = value;
        }
        else if(inRange(accessoryControl.uid(), UID::Range::accessoryDCC))
        {
          channel = OutputChannel::AccessoryDCC;
          address = 1 + (accessoryControl.uid() - UID::Range::accessoryDCC.first);
          //if(address > m_outputValuesDCC.size() || m_outputValuesDCC[address - 1] == value)
          //  break;
          //m_outputValuesDCC[address - 1] = value;
        }
        else
        {
          break;
        }

        EventLoop::call(
          [this, channel, address, value]()
          {
            m_outputController->updateOutputValue(channel, address, value);
          });
      }
      break;

    case Command::AccessoryConfig:
    case Command::S88Polling:
      // not (yet) implemented
      break;

    case Command::FeedbackEvent:
      if(message.dlc == 8)
      {
        if(m_inputController)
        {
          const auto& feedbackState = static_cast<const FeedbackState&>(message);

          if(feedbackState.deviceId() == 0) //! \todo what about other values?
          {
            const auto value = feedbackState.stateNew() == 0 ? TriState::False : TriState::True;
            if(inRange(feedbackState.contactId(), s88AddressMin, s88AddressMax) && m_inputValues[feedbackState.contactId() - s88AddressMin] != value)
            {
              m_inputValues[feedbackState.contactId() - s88AddressMin] = value;

              EventLoop::call(
                [this, address=feedbackState.contactId(), value]()
                {
                  m_inputController->updateInputValue(InputController::defaultInputChannel, address, value);
                });
            }
          }
        }
      }
      break;

    case Command::SX1Event:
      // not (yet) implemented
      break;

    case Command::Ping:
      if(message.dlc == 0 && !message.isResponse())
      {
        send(PingReply(m_config.nodeUID, TRAINTASTIC_VERSION_MAJOR, TRAINTASTIC_VERSION_MINOR, DeviceId::Traintastic));
      }
      else if(message.dlc == 8 && message.isResponse())
      {
        const auto& pingReply = static_cast<const PingReply&>(message);

        if(auto it = m_nodes.find(pingReply.uid()); it == m_nodes.end())
        {
          if(it == m_nodes.end()) // new node
          {
            Node node;
            node.uid = pingReply.uid();
            node.softwareVersionMajor = pingReply.softwareVersionMajor();
            node.softwareVersionMinor = pingReply.softwareVersionMinor();
            node.deviceId = pingReply.deviceId();
            m_nodes.emplace(pingReply.uid(), node);
          }

          if(pingReply.uid() != m_config.nodeUID)
          {
            // queue the requests and wait for some time before sending them.
            // multiple transfer don't work well at the same time.
            m_statusDataConfigRequestQueue.emplace(pingReply.uid(), 0);
            restartStatusDataConfigTimer();
          }
        }
      }
      break;

    case Command::Update:
    case Command::ReadConfigData:
    case Command::BootloaderCAN:
    case Command::BootloaderTrack:
      // not (yet) implemented
      break;

    case Command::StatusDataConfig:
      if(message.dlc == 5 && !message.isResponse() && static_cast<const UidMessage&>(message).uid() == m_config.nodeUID)
      {
        const uint32_t uid = static_cast<const UidMessage&>(message).uid();
        const uint8_t index = message.data[4];
        switch(index)
        {
          case 0x00:
          {
            StatusData::DeviceDescription desc;
            desc.serialNumber = m_config.nodeSerialNumber;
            memcpy(desc.articleNumber, nodeArticleNumber.data(), std::min(nodeArticleNumber.size(), sizeof(desc.articleNumber)));
            desc.deviceName = nodeDeviceName;
            for(const auto& reply : statusDataConfigReply(m_config.nodeUID, uid, index, desc))
              send(reply);
            break;
          }
        }
      }
      else if(message.dlc == 5 && message.isResponse())
      {
        const auto& configReply = static_cast<const StatusDataConfig&>(message);
        receiveStatusDataConfig(configReply.uid(), configReply.index(), m_statusConfigData);
        m_statusConfigData.clear();
      }
      else if(message.dlc == 6 && message.isResponse())
      {
        const auto& configReply = static_cast<const StatusDataConfigReply&>(message);
        if(m_statusConfigData.size() / 8 == configReply.packetCount())
        {
          receiveStatusDataConfig(configReply.uid(), configReply.index(), m_statusConfigData);
        }
        m_statusConfigData.clear();
      }
      else if(message.dlc == 8 && message.isResponse())
      {
        const auto& configData = static_cast<const StatusDataConfigReplyData&>(message);
        if(m_statusConfigData.empty() && configData.hash() == StatusDataConfigReplyData::startHash)
        {
          m_statusConfigData.resize(8);
          std::memcpy(m_statusConfigData.data(), configData.data, 8);
        }
        else if((m_statusConfigData.size() / 8 + StatusDataConfigReplyData::startHash) == configData.hash())
        {
          m_statusConfigData.resize(m_statusConfigData.size() + 8);
          std::memcpy(m_statusConfigData.data() + m_statusConfigData.size() - 8, configData.data, 8);
        }
        else
          m_statusConfigData.clear(); // invalid data -> reset
      }
      break;

    case Command::ConfigData:
      if(message.isResponse() && message.dlc == 8)
      {
        m_configDataStreamCollector = std::make_unique<ConfigDataStreamCollector>(std::string{static_cast<const ConfigData&>(message).name()});
      }
      break;

    case Command::ConfigDataStream:
      if(m_configDataStreamCollector) /*[[likely]]*/
      {
        const auto status = m_configDataStreamCollector->process(static_cast<const ConfigDataStream&>(message));
        if(status != ConfigDataStreamCollector::Collecting)
        {
          if(status == ConfigDataStreamCollector::Complete)
          {
            receiveConfigData(std::move(m_configDataStreamCollector));
          }
          else // error
          {
            m_configDataStreamCollector.reset();
          }
        }
      }
      break;
  }
}

void Kernel::systemStop()
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this]()
    {
      send(SystemStop());
    });
}

void Kernel::systemGo()
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this]()
    {
      send(SystemGo());
    });
}

void Kernel::systemHalt()
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this]()
    {
        send(SystemHalt());
    });
}

void Kernel::getLocomotiveList()
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this]()
    {
      send(ConfigData(m_config.nodeUID, ConfigDataName::loks));
    });
}

void Kernel::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  assert(isEventLoopThread());
  uint32_t uid = 0;

  switch(decoder.protocol.value())
  {
    case DecoderProtocol::DCCShort:
    case DecoderProtocol::DCCLong:
      uid = UID::locomotiveDCC(decoder.address);
      break;

    case DecoderProtocol::Motorola:
      uid = UID::locomotiveMotorola(decoder.address);
      break;

    case DecoderProtocol::MFX:
      if(const auto& it = m_mfxUIDtoSID.find(decoder.mfxUID); it != m_mfxUIDtoSID.end())
      {
        uid = UID::locomotiveMFX(it->second);
      }
      else
      {
        Log::log(logId, LogMessage::E2024_UNKNOWN_LOCOMOTIVE_MFX_UID_X, toHex(decoder.mfxUID.value()));
      }
      break;

    case DecoderProtocol::None:
    case DecoderProtocol::Selectrix:
      assert(false);
      break;
  }

  if(uid == 0)
    return;

  if(has(changes, DecoderChangeFlags::Direction))
  {
    LocomotiveDirection::Direction direction = LocomotiveDirection::Direction::Same;

    switch(decoder.direction.value())
    {
      case Direction::Forward:
        direction = LocomotiveDirection::Direction::Forward;
        break;

      case Direction::Reverse:
        direction = LocomotiveDirection::Direction::Reverse;
        break;

      case Direction::Unknown:
        break;
    }

    if(direction != LocomotiveDirection::Direction::Same)
      postSend(LocomotiveDirection(uid, direction));
  }

  if(has(changes, DecoderChangeFlags::EmergencyStop) && decoder.emergencyStop)
    postSend(LocomotiveEmergencyStop(uid));
  else if(has(changes, DecoderChangeFlags::Throttle | DecoderChangeFlags::EmergencyStop))
    postSend(LocomotiveSpeed(uid, Decoder::throttleToSpeedStep(decoder.throttle, LocomotiveSpeed::speedMax)));

  if(has(changes, DecoderChangeFlags::FunctionValue) && functionNumber <= std::numeric_limits<uint8_t>::max())
    postSend(LocomotiveFunction(uid, functionNumber, decoder.getFunctionValue(functionNumber)));
}

bool Kernel::setOutput(OutputChannel channel, uint16_t address, OutputPairValue value)
{
  assert(isEventLoopThread());
  assert(value == OutputPairValue::First || value == OutputPairValue::Second);

  m_ioContext.post(
    [this, channel, address, value]()
    {
      uint32_t uid = 0;

      switch(channel)
      {
        case OutputChannel::AccessoryMotorola:
          assert(inRange(address, Motorola::Accessory::addressMin, Motorola::Accessory::addressMax));
          if(m_outputValuesMotorola[address - Motorola::Accessory::addressMin] == value)
            return;
          uid = MarklinCAN::UID::accessoryMotorola(address);
          break;

        case OutputChannel::AccessoryDCC:
          assert(inRange(address, DCC::Accessory::addressMin, DCC::Accessory::addressMax));
          if(m_outputValuesDCC[address - DCC::Accessory::addressMin] == value)
            return;
          uid = MarklinCAN::UID::accessoryDCC(address);
          break;

        default: /*[[unlikely]]*/
          assert(false);
          return;
      }
      assert(uid != 0);

      MarklinCAN::AccessoryControl cmd(uid);
      cmd.setPosition(value == OutputPairValue::First ? MarklinCAN::AccessoryControl::positionOff : MarklinCAN::AccessoryControl::positionOn);
      //cmd.setCurrent(value ? 1 : 0);
      send(cmd);
    });

  return true;
}

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(isEventLoopThread());
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

void Kernel::send(const Message& message)
{
  assert(isKernelThread());

  if(m_config.debugLogRXTX)
    EventLoop::call([this, msg=toString(message)](){ Log::log(logId, LogMessage::D2001_TX_X, msg); });

  m_ioHandler->send(message);
}

void Kernel::postSend(const Message& message)
{
  m_ioContext.post(
    [this, message]()
    {
      send(message);
    });
}

void Kernel::receiveStatusDataConfig(uint32_t nodeUID, uint8_t index, const std::vector<std::byte>& statusConfigData)
{
  auto it = m_nodes.find(nodeUID);
  if(it == m_nodes.end())
    return;

  Node& node = it->second;

  if(index == 0)
  {
    const auto devDesc = StatusData::DeviceDescription::decode(statusConfigData);
    node.serialNumber = devDesc.serialNumber;
    node.articleNumber.assign(devDesc.articleNumber, strnlen(devDesc.articleNumber, sizeof(devDesc.articleNumber)));
    node.deviceName = devDesc.deviceName;
    node.numberOfReadings = devDesc.numberOfReadings;
    node.numberOfConfigurationChannels = devDesc.numberOfConfigurationChannels;

    // schedule read of reading/configuration descriptions:
    const uint8_t lastIndex = devDesc.numberOfReadings + devDesc.numberOfConfigurationChannels;
    for(uint8_t i = 1; i <= lastIndex; i++)
      m_statusDataConfigRequestQueue.emplace(node.uid, i);
  }
  else if(index <= node.numberOfReadings)
  {
    node.readings.emplace_back(StatusData::ReadingDescription::decode(statusConfigData));
  }
  else if(index <= node.numberOfReadings + node.numberOfConfigurationChannels)
  {
    node.configurations.emplace_back(StatusData::ConfigurationDescription::decode(statusConfigData));
  }

  if(index == node.numberOfReadings + node.numberOfConfigurationChannels)
  {
    nodeChanged(node);
  }

  if(!m_statusDataConfigRequestQueue.empty() && m_statusDataConfigRequestQueue.front().uid == nodeUID && m_statusDataConfigRequestQueue.front().index == index)
  {
    m_statusDataConfigRequestQueue.pop();
    m_statusDataConfigRequestRetries = statusDataConfigRequestRetryCount;
    if(!m_statusDataConfigRequestQueue.empty())
      restartStatusDataConfigTimer();
    else if(m_state == State::DiscoverNodes)
      nextState();
  }
}

void Kernel::receiveConfigData(std::unique_ptr<ConfigDataStreamCollector> configData)
{
  const auto basename = m_debugDir / logId / "configstream" / configData->name;
  if(m_config.debugConfigStream)
  {
    writeFile(std::filesystem::path(basename).concat(".bin"), configData->bytes());
  }

  if(configData->name == ConfigDataName::loks)
  {
    const size_t uncompressedSize = be_to_host(*reinterpret_cast<const uint32_t*>(configData->data()));
    std::string locList;
    if(ZLib::Uncompress::toString(configData->data() + sizeof(uint32_t), configData->dataSize() - sizeof(uint32_t), uncompressedSize, locList))
    {
      if(m_config.debugConfigStream)
      {
        writeFile(std::filesystem::path(basename).concat(".txt"), locList);
      }

      EventLoop::call(
        [this, list=std::make_shared<LocomotiveList>(locList)]()
        {
          // update MFX UID to SID list:
          m_mfxUIDtoSID.clear();
          for(const auto& item : *list)
            m_mfxUIDtoSID.emplace(item.mfxUID, item.sid);

          if(m_onLocomotiveListChanged) /*[[likely]]*/
            m_onLocomotiveListChanged(list);
        });
    }

    if(m_state == State::DownloadLokList)
      nextState();
  }
}

void Kernel::restartStatusDataConfigTimer()
{
  assert(!m_statusDataConfigRequestQueue.empty());
  m_statusDataConfigRequestTimer.cancel();
  m_statusDataConfigRequestTimer.expires_after(std::chrono::milliseconds(50));
  m_statusDataConfigRequestTimer.async_wait(
    [this](const boost::system::error_code& ec)
    {
      if(!ec)
      {
        if(!m_statusConfigData.empty())
          return; // if in progress, don't request, when finished it will start the timer again

        if(m_statusDataConfigRequestRetries > 0) /*[[likely]]*/
          m_statusDataConfigRequestRetries--;

        if(m_statusDataConfigRequestRetries == 0)
        {
          // give up, no response

          if(auto it = m_nodes.find(m_statusDataConfigRequestQueue.front().uid); it != m_nodes.end()) /*[[likely]]*/
          {
            nodeChanged(it->second);
          }

          m_statusDataConfigRequestQueue.pop();
          m_statusDataConfigRequestRetries = statusDataConfigRequestRetryCount;
        }
        else
        {
          const auto& request = m_statusDataConfigRequestQueue.front();
          send(StatusDataConfig(m_config.nodeUID, request.uid, request.index));
        }
      }

      if(ec != boost::asio::error::operation_aborted)
      {
        if(!m_statusDataConfigRequestQueue.empty())
          restartStatusDataConfigTimer();
        else if(m_state == State::DiscoverNodes)
          nextState();
      }
    });
}

void Kernel::nodeChanged(const Node& node)
{
  assert(isKernelThread());

  if(m_onNodeChanged) /*[[likely]]*/
    EventLoop::call(
      [this, node=node]()
      {
        static_assert(!std::is_reference_v<decltype(node)>);
        m_onNodeChanged(node);
      });

  if(m_config.debugStatusDataConfig)
  {
    using namespace nlohmann;

    // serialize into JSON:
    auto data = json::object();
    data["uid"] = node.uid;
    data["software_version"] = std::to_string(node.softwareVersionMajor).append(".").append(std::to_string(node.softwareVersionMinor));
    data["device_id"] = node.deviceId;
    if(!node.deviceName.empty() || node.serialNumber != 0 || !node.articleNumber.empty() || node.numberOfReadings != 0 || node.numberOfConfigurationChannels != 0)
    {
      data["serial_number"] = node.serialNumber;
      data["article_number"] = node.articleNumber;
      data["device_name"] = node.deviceName;
      data["number_of_readings"] = node.numberOfReadings;
      auto readings = json::array();
      for(const auto& reading : node.readings)
      {
        auto readingData = json::object();
        readingData["channel"] = reading.channel;
        readingData["power"] = reading.power;
        readingData["color"] = reading.color;
        readingData["zero"] = reading.zero;
        readingData["rangeEnd"] = reading.rangeEnd;
        readingData["description"] = reading.description;
        readingData["labelStart"] = reading.labelStart;
        readingData["labelEnd"] = reading.labelEnd;
        readingData["unit"] = reading.unit;
        readings.emplace_back(readingData);
      }
      data["readings"] = readings;
      data["number_of_configuration_channels"] = node.numberOfConfigurationChannels;
      auto configurations = json::array();
      for(const auto& configuration : node.configurations)
      {
        auto configurationData = json::object();
        configurationData["channel"] = configuration.channel;
        configurationData["description"] = configuration.description;
        switch(configuration.type)
        {
          case StatusData::ConfigurationDescription::Type::List:
          {
            configurationData["type"] = "list";
            configurationData["default"] = configuration.default_;
            auto items = json::array();
            for(const auto& item : configuration.listItems)
              items.emplace_back(item);
            configurationData["items"] = items;
            break;
          }
          case StatusData::ConfigurationDescription::Type::Number:
            configurationData["type"] = "number";
            configurationData["value_min"] = configuration.valueMin;
            configurationData["value_max"] = configuration.valueMax;
            configurationData["value"] = configuration.value;
            configurationData["label_start"] = configuration.labelStart;
            configurationData["label_end"] = configuration.labelEnd;
            configurationData["unit"] = configuration.unit;
            break;

          default:
            configurationData["type"] = static_cast<uint8_t>(configuration.type);
            assert(false);
            break;
        }
        configurations.emplace_back(configurationData);
      }
      data["configurations"] = configurations;
    }

    writeFileJSON(m_debugDir / logId / "statusdataconfig" / toHex(node.uid).append(".json"), data);
  }
}

void Kernel::changeState(State value)
{
  assert(isKernelThread());
  assert(m_state != value);

  m_state = value;

  switch(m_state)
  {
    case State::Initial:
      break;

    case State::DiscoverNodes:
      send(Ping());
      break;

    case State::SetAccessorySwitchTime:
      send(AccessorySwitchTime(m_config.defaultSwitchTime / 10));
      break;

    case State::DownloadLokList:
      send(ConfigData(m_config.nodeUID, ConfigDataName::loks));
      break;

    case State::Started:
      KernelBase::started();
      break;
  }
}

}
