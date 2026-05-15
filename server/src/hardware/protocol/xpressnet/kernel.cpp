/**
 * server/src/hardware/protocol/xpressnet/kernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2026 Reinder Feenstra
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
#include "messages.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../protocol/dcc/dcc.hpp"
#include "../../output/outputcontroller.hpp"
#include "../../input/inputcontroller.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"

namespace XpressNet {

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_decoderController{nullptr}
  , m_inputController{nullptr}
  , m_outputController{nullptr}
  , m_config{config}
  , m_pendingQueryTimeout{m_ioContext}
  , m_pollTimer{m_ioContext}
{
}

void Kernel::setConfig(const Config& config)
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this, newConfig=config]()
    {
      m_config = newConfig;
    });
}

void Kernel::start()
{
  assert(isEventLoopThread());

  assert(m_ioHandler);
  assert(!m_started);

  // reset all state values
  m_trackPowerOn = TriState::Undefined;
  m_emergencyStop = TriState::Undefined;
  m_inputValues.fill(TriState::Undefined);

  m_thread = std::thread(
    [this]()
    {
      setThreadName("xpressnet");
      boost::asio::executor_work_guard<decltype(m_ioContext.get_executor())> work{m_ioContext.get_executor()};
      m_ioContext.run();
    });

  boost::asio::post(m_ioContext, 
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
        return;
      }
    });

#ifndef NDEBUG
  m_started = true;
#endif
}

void Kernel::stop()
{
  assert(isEventLoopThread());

  boost::asio::post(m_ioContext,
    [this]()
    {
      boost::system::error_code ec;
      m_pendingQueryTimeout.cancel(ec);
      m_pollTimer.cancel(ec);

      m_ioHandler->stop();

      m_ioContext.stop();
    });

  m_thread.join();

#ifndef NDEBUG
  m_started = false;
#endif
}

void Kernel::started()
{
  assert(isKernelThread());

  KernelBase::started();

  send(QueryCentralVersion());

  pollDecoders();
}

void Kernel::receive(const Message& message)
{
  assert(isKernelThread());

  if(m_config.debugLogRXTX)
  {
    Utils::PendingQuery optAddress;
    if(!m_pendingQueries.empty())
      optAddress = m_pendingQueries.at(0);
    EventLoop::call(
      [this, msg=toString(message, false, optAddress)]()
      {
        Log::log(logId, LogMessage::D2002_RX_X, msg);
      });
  }

  switch(message.identification())
  {
    case idFeedbackBroadcast:
    {
      const auto* feedback = static_cast<const FeedbackBroadcast*>(&message);

      for(uint8_t i = 0; i < feedback->pairCount(); i++)
      {
        const FeedbackBroadcast::Pair& pair = feedback->pair(i);
        switch(pair.type())
        {
          case FeedbackBroadcast::Pair::Type::AccessoryDecoderWithoutFeedback:
          case FeedbackBroadcast::Pair::Type::AccessoryDecoderWithFeedback:
          {
            if(m_outputController)
            {
              const uint16_t baseAddress = pair.groupAddress() * 2;

              for(uint16_t j = 0; j < 2; j++)
              {
                const uint16_t fullAddress = baseAddress + j;
                const uint8_t status = (pair.statusNibble() >> (j * 2)) & 0x03;

                OutputPairValue value = OutputPairValue::Undefined;
                if(status == 1)
                  value = OutputPairValue::First;
                else if(status == 2)
                  value = OutputPairValue::Second;

                EventLoop::call(
                  [this, address=1 + fullAddress, value]()
                  {
                    m_outputController->updateOutputValue(OutputChannel::Accessory, OutputAddress(address), value);
                  });
              }
            }
            break;
          }
          case FeedbackBroadcast::Pair::Type::FeedbackModule:
          {
            if(m_inputController)
            {
              const uint16_t baseAddress = pair.groupAddress() << 2;

              for(uint16_t j = 0; j < 4; j++)
              {
                const uint16_t fullAddress = baseAddress + j;
                const TriState value = toTriState((pair.statusNibble() & (1 << j)) != 0);
                if(m_inputValues[fullAddress] != value)
                {
                  if(m_config.debugLogInput)
                    EventLoop::call(
                      [this, address=1 + fullAddress, value]()
                      {
                        Log::log(logId, LogMessage::D2007_INPUT_X_IS_X, address, value == TriState::True ? std::string_view{"1"} : std::string_view{"0"});
                      });

                  m_inputValues[fullAddress] = value;

                  EventLoop::call(
                    [this, address=1 + fullAddress, value]()
                    {
                      m_inputController->updateInputValue(InputChannel::Input, InputAddress(address), value);
                    });
                }
              }
            }
            break;
          }
          case FeedbackBroadcast::Pair::Type::ReservedForFutureUse:
            break;
        }
      }

      break;
    }
    case 0x60:
    {
      if(message == TrackPowerOff())
      {
        EventLoop::call(
          [this]()
          {
            if(m_trackPowerOn != TriState::False)
            {
              m_trackPowerOn = TriState::False;
              m_emergencyStop = TriState::False;
              if(m_onTrackPowerChanged)
                m_onTrackPowerChanged(false, false);
            }
          });
      }
      else if(message == NormalOperationResumed())
      {
        EventLoop::call(
          [this]()
          {
            if(m_trackPowerOn != TriState::True || m_emergencyStop != TriState::False)
            {
              m_trackPowerOn = TriState::True;
              m_emergencyStop = TriState::False;
              if(m_onTrackPowerChanged)
                m_onTrackPowerChanged(true, false);
            }
          });
      }
      else if(message.header == REPLY_VERSION_2_3)
      {
        const auto& reply = static_cast<const CentralVersionReplyV1&>(message);
        if(reply.db1 == idCentralVersion)
        {
          setCentralVersion(reply.versionHex, HardwareType::HWT_UNKNOWN);
        }
        else if(reply.db1 == idCentralStatusReply)
        {
          const auto& statusReply = static_cast<const CentralStatusReply&>(message);
          bool isEStop = has(statusReply.status, CentralStatusFlags::EStop);
          bool isPowerOn = !has(statusReply.status, CentralStatusFlags::PowerOff);

          EventLoop::call(
            [this, isEStop, isPowerOn]()
            {
              if(m_trackPowerOn != isPowerOn || m_emergencyStop != isEStop)
              {
                m_trackPowerOn = toTriState(isPowerOn);
                m_emergencyStop = toTriState(isEStop);
                if(m_onTrackPowerChanged)
                  m_onTrackPowerChanged(isPowerOn, isEStop);
              }
            });
        }
      }
      else if(message.header == REPLY_VERSION_3_0)
      {
        const auto& reply = static_cast<const CentralVersionReplyV3&>(message);
        if(reply.db1 == idCentralVersion)
        {
          setCentralVersion(reply.versionHex, reply.commandStationId());
        }
      }
      break;
    }
    case 0x80:
    {
      if(message == EmergencyStop())
      {
        EventLoop::call(
          [this]()
          {
            if(m_emergencyStop != TriState::True)
            {
              m_emergencyStop = TriState::True;
              m_trackPowerOn = TriState::True;

              if(m_onTrackPowerChanged)
                m_onTrackPowerChanged(true, true);
            }
          });
      }
      break;
    }
    case 0xE0:
    {
      switch(message.header)
      {
      case GET_LOCO_INFO:
      {
        const auto& locoInstr = static_cast<const LocomotiveInstruction&>(message);
        if(locoInstr.identification == idLocomotiveBusy)
        {
          auto loco = std::find_if(m_locomotives.begin(), m_locomotives.end(),
            [address=locoInstr.address()](const Locomotive &item) -> bool
            {
              return item.address == address;
            });

          if(loco == m_locomotives.end())
            break;

          loco->flags |= Locomotive::Flags::OwnedByXBus;

          // Immediately start querying
          postQuery({locoInstr.address(),
                     m_config.useRocoF13F20Command ?
                         Utils::PendingQuery::ROCOCumulativeLocoInfo :
                         Utils::PendingQuery::LocoInfoAndF0F12});
          break;
        }
        else if(locoInstr.identification == idReplyFuncF13F28)
        {
          const auto& funcInfo13 = static_cast<const FunctionInfoF13F28&>(message);
          const uint16_t replyAddress = popAddressQuerySendNext(Utils::PendingQuery::FuncInfoF13F28);
          if(!replyAddress)
            break; // We did not ask for function info, ignore it

          // After receiving basic loco info, query super-higher functions
          for(const Locomotive &loco : std::as_const(m_locomotives))
          {
            if(loco.address != replyAddress)
              continue;

            if((loco.flags & Locomotive::Flags::HasF29F68) == Locomotive::Flags::HasF29F68)
              postQuery({replyAddress, Utils::PendingQuery::FuncInfoF29F68});
            break;
          }

          EventLoop::call(
            [this, replyAddress, funcInfoCopy=funcInfo13]()
            {
              try
              {
                if(auto decoder = m_decoderController->getDecoder(DCC::getProtocol(replyAddress), replyAddress))
                {
                  //Function get always updated because we do not store a copy in cache
                  //so there is no way to tell in advance if they changed
                  for(int i = 13; i <= 28; i++)
                  {
                    decoder->setFunctionValue(i, funcInfoCopy.getFunction(i));
                  }
                }
              }
              catch(...)
              {

              }
            });
          break;
        }
        break;
      }
      case SET_LOCO:
      {
        const auto& locoInstr = static_cast<const LocomotiveInstruction&>(message);
        if((locoInstr.identification & LocomotiveInfo::identificationMask) == 0)
        {
          const auto& locoInfo = static_cast<const LocomotiveInfo&>(message);
          handleLocoInfoReply(locoInfo);
          break;
        }

        break;
      }
      case FUNC_INFO_V4:
      {
        const auto& funcInfo29 = static_cast<const FunctionInfoF29F68&>(message);
        if(funcInfo29.identification != idReplyFuncF29F68)
          break; // Not a F29F68 info message

        const uint16_t replyAddress = popAddressQuerySendNext(Utils::PendingQuery::FuncInfoF29F68);
        if(!replyAddress)
          break; // We did not ask for function info, ignore it

        EventLoop::call(
          [this, replyAddress, funcInfoCopy=funcInfo29]()
          {
            try
            {
              if(auto decoder = m_decoderController->getDecoder(DCC::getProtocol(replyAddress), replyAddress))
              {
                //Function get always updated because we do not store a copy in cache
                //so there is no way to tell in advance if they changed
                for(int i = 29; i <= 68; i++)
                {
                  decoder->setFunctionValue(i, funcInfoCopy.getFunction(i));
                }
              }
            }
            catch(...)
            {

            }
          });

        break;
      }
      case LOCO_INFO_CUMULATIVE:
      {
        const auto& locoInfo = static_cast<const RocoMultiMAUS::LocomotiveCumulativeInfo&>(message);
        if((locoInfo.identification & RocoMultiMAUS::LocomotiveCumulativeInfo::identificationMask) == 0)
        {
          handleLocoInfoReply(locoInfo);
          break;
        }

        break;
      }
      }
      break;
    }
  }
}

void Kernel::resumeOperations()
{
  assert(isEventLoopThread());

  if(m_trackPowerOn != TriState::True || m_emergencyStop != TriState::False)
  {
    boost::asio::post(m_ioContext, 
      [this]()
      {
        send(ResumeOperationsRequest());
      });
  }
}

void Kernel::stopOperations()
{
  assert(isEventLoopThread());

  if(m_trackPowerOn != TriState::False || m_emergencyStop != TriState::False)
  {
    boost::asio::post(m_ioContext, 
      [this]()
      {
        send(StopOperationsRequest());
      });
  }
}

void Kernel::stopAllLocomotives()
{
  assert(isEventLoopThread());

  if(m_trackPowerOn != TriState::True || m_emergencyStop != TriState::True)
  {
    boost::asio::post(m_ioContext, 
      [this]()
      {
        send(StopAllLocomotivesRequest());
      });
  }
}

void Kernel::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  assert(isEventLoopThread());

  if(m_isUpdatingDecoderFromKernel)
  {
    // This change was caused by Xpressnet message so there is not point
    // on informing back Xpressnet with another message
    // Reset the guard to allow Train and other parts of code
    // to react to this change and further edit decoder state
    m_isUpdatingDecoderFromKernel = false;
    return;
  }

  if(m_config.useEmergencyStopLocomotiveCommand && changes == DecoderChangeFlags::EmergencyStop && decoder.emergencyStop)
  {
    postSend(EmergencyStopLocomotive(decoder.address));
  }
  else if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Direction | DecoderChangeFlags::Throttle | DecoderChangeFlags::SpeedSteps))
  {
    SpeedAndDirectionInstruction spd(
      decoder.address,
      decoder.emergencyStop,
      decoder.direction);

    spd.setSpeedSteps(decoder.speedSteps);
    //assert(spd.speedSteps() == decoder.speedSteps);

    if(!decoder.emergencyStop)
      spd.setSpeedStep(Decoder::throttleToSpeedStep<uint8_t>(decoder.throttle, decoder.speedSteps));

    if(decoder.speedSteps == 14)
      static_cast<SpeedAndDirectionInstruction14&>(spd).setFl(decoder.getFunctionValue(0));

    spd.updateChecksum();
    postSend(spd);
  }
  else if(has(changes, DecoderChangeFlags::FunctionValue))
  {
    uint8_t maxSupportedGroup = 3;
    if(m_centralVersionEventLoop >= XNet_4_0)
      maxSupportedGroup = 10;
    else if(m_centralVersionEventLoop >= XNet_3_6)
      maxSupportedGroup = 5;
    else if(m_config.useRocoF13F20Command)
      maxSupportedGroup = 4;

    for(uint8_t group = 1; group <= maxSupportedGroup; group++)
    {
      const uint8_t minIndex = FunctionInstructionGroup::getMinFunctionIndex(group);
      const uint8_t maxIndex = FunctionInstructionGroup::getMaxFunctionIndex(group);

      if(functionNumber < minIndex || functionNumber > maxIndex)
        continue;

      if(group == 4 && m_config.useRocoF13F20Command)
      {
        postSend(RocoMultiMAUS::FunctionInstructionF13F20(
          decoder.address,
          decoder.getFunctionValue(13),
          decoder.getFunctionValue(14),
          decoder.getFunctionValue(15),
          decoder.getFunctionValue(16),
          decoder.getFunctionValue(17),
          decoder.getFunctionValue(18),
          decoder.getFunctionValue(19),
          decoder.getFunctionValue(20)));
      }
      else
      {
        FunctionInstructionGroup setFunc(decoder.address, group);
        for(uint8_t i = minIndex; i <= maxIndex; i++)
          setFunc.setFunction(i, decoder.getFunctionValue(i));
        setFunc.updateChecksum();
        postSend(setFunc);
      }
      break;
    }
  }
}

bool Kernel::setOutput(uint16_t address, OutputPairValue value)
{
  assert(isEventLoopThread());
  assert(address >= accessoryOutputAddressMin && address <= accessoryOutputAddressMax);
  assert(value == OutputPairValue::First || value == OutputPairValue::Second);
  boost::asio::post(m_ioContext, 
    [this, address, value]()
    {
      if(m_centralVersion >= XNet_3_8)
      {
        send(
          AccessoryDecoderOperationRequestV3(
            m_config.useRocoAccessoryAddressing ? address + 4 : address,
            value == OutputPairValue::Second,
            true));
      }
      else if(address <= 1024)
      {
        send(
          AccessoryDecoderOperationRequestV1(
            m_config.useRocoAccessoryAddressing ? address + 4 : address,
            value == OutputPairValue::Second,
            true));
      }

    });
  return true;
}

void Kernel::simulateInputChange(uint16_t address, SimulateInputAction action)
{
  assert(isEventLoopThread());

  if(m_simulation)
    boost::asio::post(m_ioContext, 
      [this, address, action]()
      {
        if((action == SimulateInputAction::SetFalse && m_inputValues[address - 1] == TriState::False) ||
            (action == SimulateInputAction::SetTrue && m_inputValues[address - 1] == TriState::True))
          return; // no change

        const uint16_t groupAddress = (address - 1) >> 2;
        const auto index = static_cast<uint8_t>((address - 1) & 0x0003);

        std::byte message[sizeof(FeedbackBroadcast) + sizeof(FeedbackBroadcast::Pair) + 1];
        memset(message, 0, sizeof(message));
        auto* feedbackBroadcast = reinterpret_cast<FeedbackBroadcast*>(&message);
        feedbackBroadcast->header = idFeedbackBroadcast;
        feedbackBroadcast->setPairCount(1);
        auto& pair = feedbackBroadcast->pair(0);
        pair.setGroupAddress(groupAddress);
        pair.setType(FeedbackBroadcast::Pair::Type::FeedbackModule);
        for(uint8_t i = 0; i < 4; i++)
        {
          const uint16_t n = (groupAddress << 2) + i;
          if(i == index)
          {
            switch(action)
            {
              case SimulateInputAction::SetFalse:
                pair.setStatus(i, false);
                break;

              case SimulateInputAction::SetTrue:
                pair.setStatus(i, true);
                break;

              case SimulateInputAction::Toggle:
                pair.setStatus(i, m_inputValues[n] != TriState::True);
                break;
            }
          }
          else
            pair.setStatus(i, m_inputValues[n] == TriState::True);
        }
        updateChecksum(*feedbackBroadcast);

        receive(*feedbackBroadcast);
      });
}

void Kernel::setDecoderList(const std::vector<Locomotive> &locoVec)
{
  assert(isEventLoopThread());

  m_ioContext.post(
      [this, vec=locoVec]()
      {
        m_locomotives = vec;
      });
}

void Kernel::updateDecoder(uint16_t address, uint16_t newAddress, uint8_t decoderFunctions)
{
  assert(isEventLoopThread());

  m_ioContext.post(
      [this, address, newAddress, decoderFunctions]()
      {
        for(Locomotive &loco : m_locomotives)
        {
          if(loco.address != address)
            continue;

          if(newAddress != 0)
            loco.address = newAddress;

          // Do an initial poll of new functions
          loco.flags = decoderFunctions | Locomotive::OwnedByXBus;
          break;
        }
      });
}

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

void Kernel::send(const Message& message)
{
  assert(isKernelThread());

  if(m_ioHandler->send(message))
  {
    if(m_config.debugLogRXTX)
      EventLoop::call(
        [this, msg=toString(message)]()
        {
          Log::log(logId, LogMessage::D2001_TX_X, msg);
        });
  }
  else
  {} // log message and go to error state
}

bool Kernel::send(std::vector<uint8_t> message)
{
  assert(isEventLoopThread());

  // At least Header + XOR, at most Header + 15 bytes + XOR
  if(!inRange<size_t>(message.size(), 2, 0xF + 2))
  {
    return false;
  }

  Message *msg = reinterpret_cast<Message*>(message.data());
  if(message.size() != msg->size())
    return false;

  msg->updateChecksum();

  boost::asio::post(m_ioContext,
    [this, msg_=std::move(message)]()
    {
      send(*reinterpret_cast<const Message*>(msg_.data()));
    });

  return true;
}

void Kernel::postQuery(const Utils::PendingQuery &query)
{
  assert(isKernelThread());
  assert(query.address >= shortAddressMin && query.address <= longAddressMax);

  if(query.type == Utils::PendingQuery::FuncInfoF29F68 && m_centralVersion < XNet_4_0)
    return;

  if(query.type == Utils::PendingQuery::FuncInfoF13F28 && m_centralVersion < XNet_3_6)
    return;

  if(query.type == Utils::PendingQuery::ROCOCumulativeLocoInfo
      && (m_centralVersion < XNet_3_0 || !m_config.useRocoF13F20Command))
    return;

  for(const Utils::PendingQuery& other : std::as_const(m_pendingQueries))
  {
    if(other.address == query.address && other.type == query.type)
      return; // Already pending
  }

  const bool wasEmpty = m_pendingQueries.empty();
  m_pendingQueries.push_back(query);

  if(wasEmpty)
    sendCurrentQuery();
}

void Kernel::sendCurrentQuery()
{
  assert(isKernelThread());

  if(m_pendingQueries.empty())
    return;

  switch (m_pendingQueries.at(0).type)
  {
  case Utils::PendingQuery::LocoInfoAndF0F12:
  {
    send(QueryLocomotiveV3(m_pendingQueries.at(0).address));
    break;
  }
  case Utils::PendingQuery::FuncInfoF13F28:
  {
    send(QueryLocomotiveFunctions(m_pendingQueries.at(0).address, 4));
    break;
  }
  case Utils::PendingQuery::FuncInfoF29F68:
  {
    send(QueryLocomotiveFunctions(m_pendingQueries.at(0).address, 6));
    break;
  }
  case Utils::PendingQuery::ROCOCumulativeLocoInfo:
  {
    send(RocoMultiMAUS::QueryLocomotiveCumulative(m_pendingQueries.at(0).address));
    break;
  }
  default:
    break;
  }

  boost::system::error_code ec;
  m_pendingQueryTimeout.cancel(ec);
  m_pendingQueryTimeout.expires_after(boost::asio::chrono::milliseconds(800));
  m_pendingQueryTimeout.async_wait(std::bind(&Kernel::onPendingQueryTimeout, this, std::placeholders::_1));
}

void Kernel::onPendingQueryTimeout(const boost::system::error_code& ec)
{
  assert(isKernelThread());

  if(ec == boost::asio::error::operation_aborted)
    return;

  if(m_pendingQueries.empty())
    return;

  // Remove first query which did not get reply
  m_pendingQueries.erase(m_pendingQueries.begin());

  // Go on to next query
  sendCurrentQuery();
}

uint16_t Kernel::popAddressQuerySendNext(Utils::PendingQuery::QueryType type)
{
  assert(isKernelThread());

  if(m_pendingQueries.empty() || m_pendingQueries.at(0).type != type)
    return 0;

  const uint16_t address = m_pendingQueries.at(0).address;

  // Remove first query which completed succesfully
  m_pendingQueries.erase(m_pendingQueries.begin());

  // Go on to next query
  sendCurrentQuery();
  return address;
}

void Kernel::pollDecoders()
{
  assert(isKernelThread());

  m_pollTimer.expires_after(boost::asio::chrono::milliseconds(1000));
  m_pollTimer.async_wait(
    [this](const boost::system::error_code& ec)
    {
      if(!ec)
        pollDecoders();
    });

  for(const Locomotive& loco : m_locomotives)
  {
    if(loco.address == 0)
      continue; // Skip invalid addresses

    if((loco.flags & Locomotive::Flags::OwnedByXBus) == Locomotive::Flags::OwnedByXBus)
      postQuery({loco.address,
        m_config.useRocoF13F20Command ?
          Utils::PendingQuery::ROCOCumulativeLocoInfo :
          Utils::PendingQuery::LocoInfoAndF0F12});
  }
}

void Kernel::setCentralVersion(uint8_t version, uint8_t commandStationId)
{
  assert(isKernelThread());
  m_centralVersion = XBusVersion(version);
  EventLoop::call([this, version, commandStationId]()
    {
      m_centralVersionEventLoop = XBusVersion(version);

      if(m_onHardwareInfoChanged)
        m_onHardwareInfoChanged(HardwareType(commandStationId),
          Utils::xbusVersionMajor(version),
          Utils::xbusVersionMinor(version));
    });
}

template<class T>
void Kernel::handleLocoInfoReply(const T &locoInfo)
{
  constexpr auto replyType = []() -> auto {
    if constexpr(std::is_same_v<T, LocomotiveInfo>)
      return Utils::PendingQuery::LocoInfoAndF0F12;
    else if constexpr(std::is_same_v<T, RocoMultiMAUS::LocomotiveCumulativeInfo>)
      return Utils::PendingQuery::ROCOCumulativeLocoInfo;
    else
      static_assert(false, "Wrong message type");
  }();

  const uint16_t replyAddress = popAddressQuerySendNext(replyType);
  if(!replyAddress)
    return; // We did not ask for locomotive info, ignore it

  // After receiving basic loco info, query higher functions
  auto loco = std::find_if(m_locomotives.begin(), m_locomotives.end(),
    [replyAddress](const Locomotive &item) -> bool
    {
      return item.address == replyAddress;
    });

  if(loco == m_locomotives.end())
    return;

  if((loco->flags & Locomotive::Flags::HasF13F28) == Locomotive::Flags::HasF13F28)
    postQuery({replyAddress, Utils::PendingQuery::FuncInfoF13F28});
  else if((loco->flags & Locomotive::Flags::HasF29F68) == Locomotive::Flags::HasF29F68)
    postQuery({replyAddress, Utils::PendingQuery::FuncInfoF29F68});

  // Enable/disable polling for this locomotive
  // When disabling, complete last poll cycle
  if(locoInfo.isBusy())
    loco->flags |= Locomotive::Flags::OwnedByXBus;
  else
    loco->flags &= ~Locomotive::Flags::OwnedByXBus;

  EventLoop::call(
    [this, replyAddress, locoInfoCopy=locoInfo]()
    {
      try
      {
        if(auto decoder = m_decoderController->getDecoder(DCC::getProtocol(replyAddress), replyAddress))
        {
          const float throttle = Decoder::speedStepToThrottle(locoInfoCopy.speedStep(), locoInfoCopy.speedSteps());

          m_isUpdatingDecoderFromKernel = true;
          decoder->emergencyStop = locoInfoCopy.isEmergencyStop();

          m_isUpdatingDecoderFromKernel = true;
          decoder->direction = locoInfoCopy.direction();

          m_isUpdatingDecoderFromKernel = true;
          decoder->throttle = throttle;

          //Reset flag guard at end
          m_isUpdatingDecoderFromKernel = false;

          //Function get always updated because we do not store a copy in cache
          //so there is no way to tell in advance if they changed
          for(int i = 0; i <= T::functionIndexMax; i++)
          {
            decoder->setFunctionValue(i, locoInfoCopy.getFunction(i));
          }
        }
      }
      catch(...)
      {

      }
    });
}

}
