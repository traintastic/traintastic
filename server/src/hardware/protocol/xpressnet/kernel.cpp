/**
 * server/src/hardware/protocol/xpressnet/kernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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
#include "../../input/inputcontroller.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"

namespace XpressNet {

Kernel::Kernel(const Config& config, bool simulation)
  : m_ioContext{1}
  , m_simulation{simulation}
  , m_decoderController{nullptr}
  , m_inputController{nullptr}
  , m_outputController{nullptr}
  , m_config{config}
#ifndef NDEBUG
  , m_started{false}
#endif
{
}

void Kernel::setConfig(const Config& config)
{
  m_ioContext.post(
    [this, newConfig=config]()
    {
      m_config = newConfig;
    });
}

void Kernel::start()
{
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
      auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
      m_ioContext.run();
    });

  m_ioContext.post(
    [this]()
    {
      m_ioHandler->start();

      if(m_onStarted)
        EventLoop::call(
          [this]()
          {
            m_onStarted();
          });
    });

#ifndef NDEBUG
  m_started = true;
#endif
}

void Kernel::stop()
{
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

void Kernel::receive(const Message& message)
{
  if(m_config.debugLogRXTX)
    EventLoop::call(
      [this, msg=toString(message)]()
      {
        Log::log(m_logId, LogMessage::D2002_RX_X, msg);
      });

  switch(message.identification())
  {
    case idFeedbackBroadcast:
    {
      const FeedbackBroadcast* feedback = static_cast<const FeedbackBroadcast*>(&message);

      for(uint8_t i = 0; i < feedback->pairCount(); i++)
      {
        const FeedbackBroadcast::Pair& pair = feedback->pair(i);
        switch(pair.type())
        {
          case FeedbackBroadcast::Pair::Type::AccessoryDecoderWithoutFeedback:
            break; // not yet implemented

          case FeedbackBroadcast::Pair::Type::AccessoryDecoderWithFeedback:
            break; // not yet implemented

          case FeedbackBroadcast::Pair::Type::FeedbackModule:
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
                        Log::log(m_logId, LogMessage::D2007_INPUT_X_IS_X, address, value == TriState::True ? std::string_view{"1"} : std::string_view{"0"});
                      });

                  m_inputValues[fullAddress] = value;

                  EventLoop::call(
                    [this, address=1 + fullAddress, value]()
                    {
                      m_inputController->updateInputValue(InputController::defaultInputChannel, address, value);
                    });
                }
              }
            }
            break;

          case FeedbackBroadcast::Pair::Type::ReservedForFutureUse:
            break;
        }
      }

      break;
    }
    case 0x60:
      if(message == NormalOperationResumed())
      {
        if(m_trackPowerOn != TriState::True || m_emergencyStop != TriState::False)
        {
          m_trackPowerOn = TriState::True;
          m_emergencyStop = TriState::False;

          if(m_onNormalOperationResumed)
            EventLoop::call(
              [this]()
              {
                m_onNormalOperationResumed();
              });
        }
      }
      else if(message == TrackPowerOff())
      {
        if(m_trackPowerOn != TriState::False)
        {
          m_trackPowerOn = TriState::False;

          if(m_onTrackPowerOff)
            EventLoop::call(
              [this]()
              {
                m_onTrackPowerOff();
              });
        }
      }
      break;

    case 0x80:
      if(message == EmergencyStop())
      {
        if(m_emergencyStop != TriState::True)
        {
          m_emergencyStop = TriState::True;

          if(m_onEmergencyStop)
            EventLoop::call(
              [this]()
              {
                m_onEmergencyStop();
              });
        }
      }
      break;
  }
}

void Kernel::resumeOperations()
{
  m_ioContext.post(
    [this]()
    {
      if(m_trackPowerOn != TriState::True || m_emergencyStop != TriState::False)
        send(ResumeOperationsRequest());
    });
}

void Kernel::stopOperations()
{
  m_ioContext.post(
    [this]()
    {
      if(m_trackPowerOn != TriState::False)
        send(StopOperationsRequest());
    });
}

void Kernel::stopAllLocomotives()
{
  m_ioContext.post(
    [this]()
    {
      if(m_emergencyStop != TriState::True)
        send(StopAllLocomotivesRequest());
    });
}

void Kernel::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_config.useEmergencyStopLocomotiveCommand && changes == DecoderChangeFlags::EmergencyStop && decoder.emergencyStop)
  {
    postSend(EmergencyStopLocomotive(decoder.address, decoder.longAddress));
  }
  else if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Direction | DecoderChangeFlags::Throttle | DecoderChangeFlags::SpeedSteps))
  {
    switch(decoder.speedSteps)
    {
      case 14:
        postSend(SpeedAndDirectionInstruction14(
          decoder.address,
          decoder.longAddress,
          decoder.emergencyStop,
          decoder.direction,
          Decoder::throttleToSpeedStep(decoder.throttle, 14),
          decoder.getFunctionValue(0)));
        break;

      case 27:
        postSend(SpeedAndDirectionInstruction27(
          decoder.address,
          decoder.longAddress,
          decoder.emergencyStop,
          decoder.direction,
          Decoder::throttleToSpeedStep(decoder.throttle, 27)));
        break;

      case 28:
        postSend(SpeedAndDirectionInstruction28(
          decoder.address,
          decoder.longAddress,
          decoder.emergencyStop,
          decoder.direction,
          Decoder::throttleToSpeedStep(decoder.throttle, 28)));
        break;

      case 126:
      case 128:
      default:
        postSend(SpeedAndDirectionInstruction128(
          decoder.address,
          decoder.longAddress,
          decoder.emergencyStop,
          decoder.direction,
          Decoder::throttleToSpeedStep(decoder.throttle, 126)));
        break;
    }
  }
  else if(has(changes, DecoderChangeFlags::FunctionValue))
  {
    if(functionNumber <= 4)
    {
      postSend(FunctionInstructionGroup1(
        decoder.address,
        decoder.longAddress,
        decoder.getFunctionValue(0),
        decoder.getFunctionValue(1),
        decoder.getFunctionValue(2),
        decoder.getFunctionValue(3),
        decoder.getFunctionValue(4)));
    }
    else if(functionNumber <= 8)
    {
      postSend(FunctionInstructionGroup2(
        decoder.address,
        decoder.longAddress,
        decoder.getFunctionValue(5),
        decoder.getFunctionValue(6),
        decoder.getFunctionValue(7),
        decoder.getFunctionValue(8)));
    }
    else if(functionNumber <= 12)
    {
      postSend(FunctionInstructionGroup3(
        decoder.address,
        decoder.longAddress,
        decoder.getFunctionValue(9),
        decoder.getFunctionValue(10),
        decoder.getFunctionValue(11),
        decoder.getFunctionValue(12)));
    }
    else if(functionNumber <= 20)
    {
      if(m_config.useRocoF13F20Command)
      {
        postSend(RocoMultiMAUS::FunctionInstructionF13F20(
          decoder.address,
          decoder.longAddress,
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
        postSend(FunctionInstructionGroup4(
          decoder.address,
          decoder.longAddress,
          decoder.getFunctionValue(13),
          decoder.getFunctionValue(14),
          decoder.getFunctionValue(15),
          decoder.getFunctionValue(16),
          decoder.getFunctionValue(17),
          decoder.getFunctionValue(18),
          decoder.getFunctionValue(19),
          decoder.getFunctionValue(20)));
      }
    }
    else if(functionNumber <= 28)
    {
      postSend(FunctionInstructionGroup5(
        decoder.address,
        decoder.longAddress,
        decoder.getFunctionValue(21),
        decoder.getFunctionValue(22),
        decoder.getFunctionValue(23),
        decoder.getFunctionValue(24),
        decoder.getFunctionValue(25),
        decoder.getFunctionValue(26),
        decoder.getFunctionValue(27),
        decoder.getFunctionValue(28)));
    }
  }
}

bool Kernel::setOutput(uint16_t address, bool value)
{
  postSend(AccessoryDecoderOperationRequest(address - 1, value));
  return true;
}

void Kernel::simulateInputChange(uint16_t address, SimulateInputAction action)
{
  if(m_simulation)
    m_ioContext.post(
      [this, address, action]()
      {
        if((action == SimulateInputAction::SetFalse && m_inputValues[address - 1] == TriState::False) ||
            (action == SimulateInputAction::SetTrue && m_inputValues[address - 1] == TriState::True))
          return; // no change

        const uint16_t groupAddress = (address - 1) >> 2;
        const uint8_t index = static_cast<uint8_t>((address - 1) & 0x0003);

        std::byte message[sizeof(FeedbackBroadcast) + sizeof(FeedbackBroadcast::Pair) + 1];
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

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

void Kernel::send(const Message& message)
{
  if(m_ioHandler->send(message))
  {
    if(m_config.debugLogRXTX)
      EventLoop::call(
        [this, msg=toString(message)]()
        {
          Log::log(m_logId, LogMessage::D2001_TX_X, msg);
        });
  }
  else
  {} // log message and go to error state
}

}
