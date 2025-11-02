/**
 * server/src/hardware/protocol/loconet/loconet.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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
#include "iohandler/iohandler.hpp"
#include "messages.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../decoder/decodercontroller.hpp"
#include "../../input/inputcontroller.hpp"
#include "../../output/outputcontroller.hpp"
#include "../../identification/identificationcontroller.hpp"
#include "../../../utils/datetimestr.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../utils/inrange.hpp"
#include "../../../pcap/pcapfile.hpp"
#include "../../../pcap/pcappipe.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../clock/clock.hpp"
#include "../../../traintastic/traintastic.hpp"
#include "../dcc/dcc.hpp"
#include "../dcc/messages.hpp"

namespace LocoNet {

static constexpr uint8_t multiplierFreeze = 0;

static void updateDecoderSpeed(const std::shared_ptr<Decoder>& decoder, uint8_t speed)
{
  decoder->emergencyStop.setValueInternal(speed == SPEED_ESTOP);

  if(speed == SPEED_STOP || speed == SPEED_ESTOP)
    decoder->throttle.setValueInternal(Decoder::throttleStop);
  else
  {
    speed--; // decrement one for ESTOP: 2..127 -> 1..126
    const auto currentStep = Decoder::throttleToSpeedStep<uint8_t>(decoder->throttle.value(), SPEED_MAX - 1);
    if(currentStep != speed) // only update trottle if it is a different step
      decoder->throttle.setValueInternal(Decoder::speedStepToThrottle<uint8_t>(speed, SPEED_MAX - 1));
  }
}

constexpr Kernel::Priority& operator ++(Kernel::Priority& value)
{
  return (value = static_cast<Kernel::Priority>(static_cast<std::underlying_type_t<Kernel::Priority>>(value) + 1));
}

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_waitingForEcho{false}
  , m_waitingForEchoTimer{m_ioContext}
  , m_waitingForResponse{false}
  , m_waitingForResponseTimer{m_ioContext}
  , m_fastClockSyncTimer(m_ioContext)
  , m_decoderController{nullptr}
  , m_inputController{nullptr}
  , m_outputController{nullptr}
  , m_identificationController{nullptr}
  , m_debugDir{Traintastic::instance->debugDir()}
  , m_config{config}
{
  assert(isEventLoopThread());
}

Kernel::~Kernel() = default;

void Kernel::setConfig(const Config& config)
{
  assert(isEventLoopThread());

  switch(config.fastClock)
  {
    case LocoNetFastClock::Off:
      disableClockEvents();
      break;

    case LocoNetFastClock::Master:
      enableClockEvents();
      break;
  }

  m_ioContext.post(
    [this, newConfig=config]()
    {
      if(newConfig.pcap != m_config.pcap)
      {
        if(newConfig.pcap)
          startPCAP(newConfig.pcapOutput);
        else
          m_pcap.reset();
      }
      else if(newConfig.pcap && newConfig.pcapOutput != m_config.pcapOutput)
      {
        m_pcap.reset();
        startPCAP(newConfig.pcapOutput);
      }

      if(newConfig.listenOnly && !m_config.listenOnly)
      {
        for(auto& queue : m_sendQueue)
          queue.clear();

        EventLoop::call(
          [this]()
          {
            Log::log(logId, LogMessage::N2006_LISTEN_ONLY_MODE_ACTIVATED);
          });
      }
      else if(!newConfig.listenOnly && m_config.listenOnly)
      {
        EventLoop::call(
          [this]()
          {
            Log::log(logId, LogMessage::N2007_LISTEN_ONLY_MODE_DEACTIVATED);
          });
      }

      if(m_config.fastClock == LocoNetFastClock::Master && newConfig.fastClock == LocoNetFastClock::Off)
      {
        setFastClockMaster(false);
      }
      else if(m_config.fastClock == LocoNetFastClock::Off && newConfig.fastClock == LocoNetFastClock::Master)
      {
        setFastClockMaster(true);
      }

      if(!m_config.fastClockSyncEnabled && newConfig.fastClockSyncEnabled)
      {
        send(RequestSlotData(SLOT_FAST_CLOCK));
        startFastClockSyncTimer();
      }
      else if(m_config.fastClockSyncEnabled && !newConfig.fastClockSyncEnabled)
      {
        stopFastClockSyncTimer();
      }
      m_config = newConfig;
    });
}

void Kernel::setOnGlobalPowerChanged(std::function<void(bool)> callback)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_onGlobalPowerChanged = std::move(callback);
}

void Kernel::setOnIdle(std::function<void()> callback)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_onIdle = std::move(callback);
}

void Kernel::setClock(std::shared_ptr<Clock> clock)
{
  assert(isEventLoopThread());
  assert(!m_started);
  assert(clock);
  m_clock = std::move(clock);
  m_fastClock.store(FastClock{m_clock->running ? m_clock->multiplier : multiplierFreeze, m_clock->hour, m_clock->minute});
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

void Kernel::setIdentificationController(IdentificationController* identificationController)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_identificationController= identificationController;
}

void Kernel::start()
{
  assert(isEventLoopThread());
  assert(m_ioHandler);
  assert(!m_started);

  // reset all state values
  m_globalPower = TriState::Undefined;
  m_emergencyStop = TriState::Undefined;
  m_addressToSlot.clear();
  m_slots.clear();
  m_pendingSlotMessages.clear();
  m_inputValues.fill(TriState::Undefined);
  m_outputValues.fill(OutputPairValue::Undefined);

  if(m_config.listenOnly)
    Log::log(logId, LogMessage::N2006_LISTEN_ONLY_MODE_ACTIVATED);

  m_thread = std::thread(
    [this]()
    {
      setThreadName("loconet");
      auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
      m_ioContext.run();
    });

  if(m_config.fastClock == LocoNetFastClock::Master)
    enableClockEvents();

  m_ioContext.post(
    [this]()
    {
      if(m_config.pcap)
        startPCAP(m_config.pcapOutput);

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

  disableClockEvents();

  m_ioContext.post(
    [this]()
    {
      m_waitingForEchoTimer.cancel();
      m_waitingForResponseTimer.cancel();
      m_fastClockSyncTimer.cancel();
      m_ioHandler->stop();
      m_pcap.reset();
    });

  m_ioContext.stop();

  m_thread.join();

#ifndef NDEBUG
  m_started = false;
#endif
}

void Kernel::started()
{
  assert(isKernelThread());

  if(m_config.fastClock == LocoNetFastClock::Master)
    setFastClockMaster(true);

  if(m_config.fastClockSyncEnabled)
    startFastClockSyncTimer();

  for(uint8_t slot = SLOT_LOCO_MIN; slot <= m_config.locomotiveSlots; slot++)
    send(RequestSlotData(slot), LowPriority);

  KernelBase::started();
}

void Kernel::receive(const Message& message)
{
  assert(isKernelThread());
  assert(isValid(message)); // only valid messages should be received

  if(m_pcap)
    m_pcap->writeRecord(&message, message.size());

  if(m_config.debugLogRXTX)
    EventLoop::call([this, msg=toString(message)](){ Log::log(logId, LogMessage::D2002_RX_X, msg); });

  bool isResponse = false;
  if(m_waitingForEcho && message == lastSentMessage())
  {
    m_waitingForEcho = false;
    m_waitingForEchoTimer.cancel();
    if(!m_waitingForResponse)
    {
      m_sendQueue[m_sentMessagePriority].pop();
      sendNextMessage();
    }
  }
  else if(m_waitingForResponse)
  {
    isResponse = isValidResponse(lastSentMessage(), message);
  }

  switch(message.opCode)
  {
    case OPC_GPON:
      if(m_globalPower != TriState::True)
      {
        m_globalPower = TriState::True;
        if(m_onGlobalPowerChanged)
          EventLoop::call(
            [this]()
            {
              m_onGlobalPowerChanged(true);
            });
      }
      break;

    case OPC_GPOFF:
      if(m_globalPower != TriState::False)
      {
        m_globalPower = TriState::False;
        if(m_onGlobalPowerChanged)
          EventLoop::call(
            [this]()
            {
              m_onGlobalPowerChanged(false);
            });
      }
      break;

    case OPC_IDLE:
      if(m_emergencyStop != TriState::True)
      {
        m_emergencyStop = TriState::True;
        if(m_onIdle)
          EventLoop::call(
            [this]()
            {
              m_onIdle();
            });
      }
      break;

    case OPC_LOCO_SPD:
      if(m_decoderController)
      {
        const auto& locoSpd = static_cast<const LocoSpd&>(message);
        if(LocoSlot* slot = getLocoSlot(locoSpd.slot))
        {
          if(!slot->isAddressValid())
          {
            send(RequestSlotData(locoSpd.slot));
          }
          else if(slot->speed != locoSpd.speed)
          {
            slot->speed = locoSpd.speed;

            EventLoop::call(
              [this, address=slot->address, speed=slot->speed]()
              {
                if(auto decoder = getDecoder(address))
                  updateDecoderSpeed(decoder, speed);
              });
          }
        }
      }
      break;

    case OPC_LOCO_DIRF: // direction and F0-F4
      if(m_decoderController)
      {
        const auto& locoDirF = static_cast<const LocoDirF&>(message);
        if(LocoSlot* slot = getLocoSlot(locoDirF.slot))
        {
          if(!slot->isAddressValid())
          {
            send(RequestSlotData(locoDirF.slot));
          }
          else
          {
            if(slot->direction != locoDirF.direction())
            {
              slot->direction = locoDirF.direction();

              EventLoop::call(
                [this, address=slot->address, direction=locoDirF.direction()]()
                {
                  if(auto decoder = getDecoder(address))
                    decoder->direction.setValueInternal(direction);
                });
            }

            updateFunctions<0, 4>(*slot, locoDirF);
          }
        }
      }
      break;

    case OPC_LOCO_SND: // F5-F8
      if(m_decoderController)
      {
        const auto& locoSnd = static_cast<const LocoSnd&>(message);
        if(LocoSlot* slot = getLocoSlot(locoSnd.slot))
        {
          if(!slot->isAddressValid())
          {
            send(RequestSlotData(locoSnd.slot));
          }
          else
          {
            updateFunctions<5, 8>(*slot, locoSnd);
          }
        }
      }
      break;

    case OPC_LOCO_F9F12:
      if(m_decoderController)
      {
        const auto& locoF9F12 = static_cast<const LocoF9F12&>(message);
        if(LocoSlot* slot = getLocoSlot(locoF9F12.slot))
        {
          if(!slot->isAddressValid())
          {
            send(RequestSlotData(locoF9F12.slot));
          }
          updateFunctions<9, 12>(*slot, locoF9F12);
        }
      }
      break;

    case OPC_INPUT_REP:
      if(m_inputController)
      {
        const auto& inputRep = static_cast<const InputRep&>(message);
        if(inputRep.isControlSet())
        {
          const auto value = toTriState(inputRep.value());
          if(m_inputValues[inputRep.fullAddress()] != value)
          {
            if(m_config.debugLogInput)
              EventLoop::call(
                [this, address=1 + inputRep.fullAddress(), value=inputRep.value()]()
                {
                  Log::log(logId, LogMessage::D2007_INPUT_X_IS_X, address, value ? std::string_view{"1"} : std::string_view{"0"});
                });

            m_inputValues[inputRep.fullAddress()] = value;

            EventLoop::call(
              [this, address=1 + inputRep.fullAddress(), value]()
              {
                m_inputController->updateInputValue(InputChannel::Input, address, value);
              });
          }
        }
      }
      break;

    case OPC_SW_REQ:
      if(m_outputController)
      {
        const auto& switchRequest = static_cast<const SwitchRequest&>(message);
        if(switchRequest.on())
        {
          const auto value = switchRequest.dir() ? OutputPairValue::Second : OutputPairValue::First;
          if(m_outputValues[switchRequest.address() - accessoryOutputAddressMin] != value)
          {
            m_outputValues[switchRequest.address() - accessoryOutputAddressMin] = value;

            EventLoop::call(
              [this, address=switchRequest.address(), value]()
              {
                m_outputController->updateOutputValue(OutputChannel::Accessory, address, value);
              });
          }
        }
      }
      break;

    case OPC_SW_REP:
      break; // unimplemented

    case OPC_SL_RD_DATA:
    {
      const uint8_t slot = *(reinterpret_cast<const uint8_t*>(&message) + 2);
      if(m_decoderController && isLocoSlot(slot))
      {
        const auto& slotReadData = static_cast<const SlotReadData&>(message);
        if(slotReadData.isFree())
        {
          clearLocoSlot(slotReadData.slot);
          break;
        }

        LocoSlot* locoSlot = getLocoSlot(slotReadData.slot, false);
        assert(locoSlot);

        if(!locoSlot->isAddressValid())
          m_addressToSlot[slotReadData.address()] = slot;

        bool changed = locoSlot->isAddressValid() && locoSlot->address != slotReadData.address();
        if(changed)
        {
          if(auto it = m_addressToSlot.find(locoSlot->address); it != m_addressToSlot.end() && it->second == slotReadData.slot)
            m_addressToSlot.erase(locoSlot->address);
          locoSlot->invalidate();
        }
        locoSlot->address = slotReadData.address();

        if(changed)
        {
          EventLoop::call(
            [this, address=locoSlot->address, speed=locoSlot->speed, direction=locoSlot->direction]()
            {
              if(auto decoder = getDecoder(address))
              {
                updateDecoderSpeed(decoder, speed);
                decoder->direction.setValueInternal(direction);
              }
            });
        }

        updateFunctions<0, 8>(*locoSlot, slotReadData);

        // check if there are pending slot messages
        if(auto it = m_pendingSlotMessages.find(locoSlot->address); it != m_pendingSlotMessages.end())
        {
          std::byte* p = it->second.data();
          size_t size = it->second.size();
          while(size > 0)
          {
            Message& slotMassage =  *reinterpret_cast<Message*>(p);
            setSlot(slotMassage, slot);
            updateChecksum(slotMassage);
            send(slotMassage);
            p += slotMassage.size();
            size -= slotMassage.size();
          }
          m_pendingSlotMessages.erase(it);
        }
      }
      else if(slot == SLOT_FAST_CLOCK)
      {
        // todo
      }
      else if(slot == SLOT_PROGRAMMING_TRACK)
      {
        // todo
      }
      break;
    }
    case OPC_BUSY:
      break; // unimplemented

    case OPC_LONG_ACK:
    {
      const auto& longAck = static_cast<const LongAck&>(message);
      if(longAck.respondingOpCode() == OPC_LOCO_ADR && longAck.ack1 == 0)
      {
        EventLoop::call(
          [this]()
          {
            Log::log(logId, LogMessage::C2004_CANT_GET_FREE_SLOT);
          });
      }
      else if(longAck.respondingOpCode() == OPC_RQ_SL_DATA && longAck.ack1 == 0 && lastSentMessage().opCode == OPC_RQ_SL_DATA)
      {
        const uint8_t slot = static_cast<const RequestSlotData&>(lastSentMessage()).slot;

        if(isLocoSlot(slot))
        {
          EventLoop::call(
            [this, slot]()
            {
              Log::log(logId, LogMessage::W2006_COMMAND_STATION_DOES_NOT_SUPPORT_LOCO_SLOT_X, slot);
            });
        }
        else if(slot == SLOT_FAST_CLOCK)
        {
          m_fastClockSupported = false;
          if(m_config.fastClockSyncEnabled)
            stopFastClockSyncTimer();

          EventLoop::call(
            [this, stoppedFastClockSyncTimer=m_config.fastClockSyncEnabled]()
            {
              Log::log(logId, LogMessage::W2007_COMMAND_STATION_DOES_NOT_SUPPORT_THE_FAST_CLOCK_SLOT);
              if(stoppedFastClockSyncTimer)
                Log::log(logId, LogMessage::N2003_STOPPED_SENDING_FAST_CLOCK_SYNC);
            });
        }
      }
      else if(m_lncvActive && m_onLNCVReadResponse &&
          longAck.respondingOpCode() == OPC_IMM_PACKET && longAck.ack1 == 0x7F &&
          Uhlenbrock::LNCVWrite::check(lastSentMessage()))
      {
        const auto& lncvWrite = static_cast<const Uhlenbrock::LNCVWrite&>(lastSentMessage());
        if(lncvWrite.lncv() == 0)
        {
          m_lncvModuleAddress = lncvWrite.value();
        }

        EventLoop::call(
          [this, lncvWrite]()
          {
            m_onLNCVReadResponse(true, lncvWrite.lncv(), lncvWrite.value());
          });
      }
      break;
    }
    case OPC_SLOT_STAT1:
      break; // unimplemented

    case OPC_CONSIST_FUNC:
      break; // unimplemented

    case OPC_UNLINK_SLOTS:
      break; // unimplemented

    case OPC_LINK_SLOTS:
      break; // unimplemented

    case OPC_MOVE_SLOTS:
      break; // unimplemented

    case OPC_RQ_SL_DATA:
      break; // unimplemented

    case OPC_SW_STATE:
      break; // unimplemented

    case OPC_SW_ACK:
      break; // unimplemented

    case OPC_LOCO_ADR:
      break; // unimplemented

    case OPC_MULTI_SENSE:
    {
      const auto& multiSense = static_cast<const MultiSense&>(message);
      if(multiSense.isTransponder())
      {
        EventLoop::call(
          [this, multiSenseTransponder=static_cast<const MultiSenseTransponder&>(multiSense)]()
          {
            m_identificationController->identificationEvent(
              IdentificationController::defaultIdentificationChannel,
              1 + multiSenseTransponder.sensorAddress(),
              multiSenseTransponder.isPresent() ? IdentificationEventType::Present : IdentificationEventType::Absent,
              multiSenseTransponder.transponderAddress(),
              Direction::Unknown,
              0);
          });
      }
      break;
    }
    case OPC_D4:
      if(m_decoderController)
      {
        const auto* bytes = reinterpret_cast<const uint8_t*>(&message);
        if(bytes[1] == 0x20)
        {
          switch(bytes[3])
          {
            case 0x08:
            {
              const auto& locoF13F19 = static_cast<const LocoF13F19&>(message);
              if(LocoSlot* slot = getLocoSlot(locoF13F19.slot))
              {
                if(!slot->isAddressValid())
                {
                  send(RequestSlotData(locoF13F19.slot));
                }
                updateFunctions<13, 19>(*slot, locoF13F19);
              }
              break;
            }
            case 0x05:
            {
              const auto& locoF12F20F28 = static_cast<const LocoF12F20F28&>(message);
              if(LocoSlot* slot = getLocoSlot(locoF12F20F28.slot))
              {
                if(!slot->isAddressValid())
                {
                  send(RequestSlotData(locoF12F20F28.slot));
                }

                const bool changed =
                  slot->functions[12] != locoF12F20F28.f12() ||
                  slot->functions[20] != locoF12F20F28.f20() ||
                  slot->functions[28] != locoF12F20F28.f28();

                if(changed)
                {
                  slot->functions[12] = toTriState(locoF12F20F28.f12());
                  slot->functions[20] = toTriState(locoF12F20F28.f20());
                  slot->functions[28] = toTriState(locoF12F20F28.f28());

                  EventLoop::call(
                    [this, address=slot->address, f12=locoF12F20F28.f12(), f20=locoF12F20F28.f20(), f28=locoF12F20F28.f28()]()
                    {
                      if(auto decoder = getDecoder(address))
                      {
                        decoder->setFunctionValue(12, f12);
                        decoder->setFunctionValue(20, f20);
                        decoder->setFunctionValue(28, f28);
                      }
                    });
                }
              }
              break;
            }
            case 0x09:
            {
              const auto& locoF21F27 = static_cast<const LocoF21F27&>(message);
              if(LocoSlot* slot = getLocoSlot(locoF21F27.slot))
              {
                if(!slot->isAddressValid())
                {
                  send(RequestSlotData(locoF21F27.slot));
                }
                updateFunctions<21, 27>(*slot, locoF21F27);
              }
              break;
            }
          }
        }
      }
      break;

    case OPC_MULTI_SENSE_LONG:
    {
      const auto& multiSense = static_cast<const MultiSenseLong&>(message);
      if(multiSense.code() == MultiSenseLong::Code::ReleaseTransponder || multiSense.code() == MultiSenseLong::Code::DetectTransponder)
      {
        EventLoop::call(
          [this, multiSense]()
          {
            m_identificationController->identificationEvent(
              IdentificationController::defaultIdentificationChannel,
              1 + multiSense.sensorAddress(),
              multiSense.code()  == MultiSenseLong::Code::DetectTransponder ? IdentificationEventType::Present : IdentificationEventType::Absent,
              multiSense.transponderAddress(),
              multiSense.transponderDirection(),
              0);
          });
      }
      break;
    }
    case OPC_E4:
      if(static_cast<const Uhlenbrock::Lissy&>(message).type() == Uhlenbrock::Lissy::Type::AddressCategoryDirection)
      {
        EventLoop::call(
          [this, lissy=static_cast<const Uhlenbrock::LissyAddressCategoryDirection&>(message)]()
          {
            m_identificationController->identificationEvent(
              IdentificationController::defaultIdentificationChannel,
              lissy.sensorAddress(),
              IdentificationEventType::Seen,
              lissy.decoderAddress(),
              lissy.direction(),
              lissy.category());
          });
      }
      break;

    case OPC_PEER_XFER:
      if(isResponse)
      {
        if(Uhlenbrock::ReadSpecialOptionReply::check(message))
        {
          [[maybe_unused]] const auto& readSpecialOptionReply = static_cast<const Uhlenbrock::ReadSpecialOptionReply&>(message);
        }
        else if(m_onLNCVReadResponse && Uhlenbrock::LNCVReadResponse::check(message))
        {
          const auto& lncvReadResponse = static_cast<const Uhlenbrock::LNCVReadResponse&>(message);
          if(lncvReadResponse.lncv() == 0)
          {
            m_lncvActive = true;
            m_lncvModuleAddress = lncvReadResponse.value();
          }

          EventLoop::call(
            [this, lncvReadResponse]()
            {
              m_onLNCVReadResponse(true, lncvReadResponse.lncv(), lncvReadResponse.value());
            });
        }
      }
      break;

    case OPC_IMM_PACKET:
      if(m_decoderController)
      {
        if(isSignatureMatch<LocoF9F12IMMShortAddress>(message))
        {
          const auto& locoF9F12 = static_cast<const LocoF9F12IMMShortAddress&>(message);
          if(LocoSlot* slot = getLocoSlotByAddress(locoF9F12.address()))
            updateFunctions<9, 12>(*slot, locoF9F12);
          else
            send(LocoAdr(locoF9F12.address()));
        }
        else if(isSignatureMatch<LocoF9F12IMMLongAddress>(message))
        {
          const auto& locoF9F12 = static_cast<const LocoF9F12IMMLongAddress&>(message);
          if(LocoSlot* slot = getLocoSlotByAddress(locoF9F12.address()))
            updateFunctions<9, 12>(*slot, locoF9F12);
          else
            send(LocoAdr(locoF9F12.address()));
        }
        else if(isSignatureMatch<LocoF13F20IMMShortAddress>(message))
        {
          const auto& locoF13F20 = static_cast<const LocoF13F20IMMShortAddress&>(message);
          if(LocoSlot* slot = getLocoSlotByAddress(locoF13F20.address()))
            updateFunctions<13, 20>(*slot, locoF13F20);
          else
            send(LocoAdr(locoF13F20.address()));
        }
        else if(isSignatureMatch<LocoF13F20IMMLongAddress>(message))
        {
          const auto& locoF13F20 = static_cast<const LocoF13F20IMMLongAddress&>(message);
          if(LocoSlot* slot = getLocoSlotByAddress(locoF13F20.address()))
            updateFunctions<13, 20>(*slot, locoF13F20);
          else
            send(LocoAdr(locoF13F20.address()));
        }
        else if(isSignatureMatch<LocoF21F28IMMShortAddress>(message))
        {
          const auto& locoF21F28 = static_cast<const LocoF21F28IMMShortAddress&>(message);
          if(LocoSlot* slot = getLocoSlotByAddress(locoF21F28.address()))
            updateFunctions<21, 28>(*slot, locoF21F28);
          else
            send(LocoAdr(locoF21F28.address()));
        }
        else if(isSignatureMatch<LocoF21F28IMMLongAddress>(message))
        {
          const auto& locoF21F28 = static_cast<const LocoF21F28IMMLongAddress&>(message);
          if(LocoSlot* slot = getLocoSlotByAddress(locoF21F28.address()))
            updateFunctions<21, 28>(*slot, locoF21F28);
          else
            send(LocoAdr(locoF21F28.address()));
        }
      }
      break; // unimplemented

    case OPC_WR_SL_DATA:
      break; // unimplemented
  }

  if(m_waitingForResponse && isResponse)
  {
    m_waitingForResponse = false;
    m_waitingForResponseTimer.cancel();
    m_sendQueue[m_sentMessagePriority].pop();
    sendNextMessage();
  }
}

void Kernel::setPowerOn(bool value)
{
  assert(isEventLoopThread());
  if(value)
  {
    m_ioContext.post(
      [this]()
      {
        if(m_globalPower != TriState::True)
          send(GlobalPowerOn(), HighPriority);
      });
  }
  else
  {
    m_ioContext.post(
      [this]()
      {
        if(m_globalPower != TriState::False)
          send(GlobalPowerOff(), HighPriority);
      });
  }
}

void Kernel::emergencyStop()
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this]()
    {
      if(m_emergencyStop != TriState::True)
        send(Idle(), HighPriority);
    });
}

void Kernel::resume()
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this]()
    {
      if(m_emergencyStop != TriState::False)
      {
        m_emergencyStop = TriState::False;

        // TODO: restore speeds
      }
    });
}

bool Kernel::send(std::span<uint8_t> packet)
{
  assert(isEventLoopThread());

  if(reinterpret_cast<Message*>(packet.data())->size() != packet.size() + 1) // verify packet length, must be all bytes excluding checksum
    return false;

  std::vector<uint8_t> data(packet.data(), packet.data() + packet.size());
  data.push_back(calcChecksum(*reinterpret_cast<Message*>(data.data())));

  if(!isValid(*reinterpret_cast<Message*>(data.data())))
    return false;

  m_ioContext.post(
    [this, message=std::move(data)]()
    {
      send(*reinterpret_cast<const Message*>(message.data()));
    });

  return true;
}

bool Kernel::immPacket(std::span<const uint8_t> dccPacket, uint8_t repeat)
{
  assert(isEventLoopThread());

  if(dccPacket.size() > ImmPacket::dccPacketSizeMax || repeat > ImmPacket::repeatMax)
    return false;

  postSend(ImmPacket(dccPacket, repeat));
  return true;
}

void Kernel::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  assert(isEventLoopThread());

  if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Throttle))
  {
    const auto speedStep = Decoder::throttleToSpeedStep<uint8_t>(decoder.throttle, SPEED_MAX - 1);
    if(m_emergencyStop == TriState::False || decoder.emergencyStop || speedStep == SPEED_STOP)
    {
      // only send speed updates if bus estop isn't active, except for speed STOP and ESTOP
      LocoSpd message{static_cast<uint8_t>(decoder.emergencyStop ? SPEED_ESTOP : (speedStep > 0 ? 1 + speedStep : SPEED_STOP))};
      postSend(decoder.address, message);
    }
  }

  if(has(changes, DecoderChangeFlags::FunctionValue | DecoderChangeFlags::Direction))
  {
    if(functionNumber <= 4 || has(changes, DecoderChangeFlags::Direction))
    {
      LocoDirF message{
        decoder.direction,
        decoder.getFunctionValue(0),
        decoder.getFunctionValue(1),
        decoder.getFunctionValue(2),
        decoder.getFunctionValue(3),
        decoder.getFunctionValue(4)};
      postSend(decoder.address, message);
    }
    else if(functionNumber <= 8)
    {
      LocoSnd message{
        decoder.getFunctionValue(5),
        decoder.getFunctionValue(6),
        decoder.getFunctionValue(7),
        decoder.getFunctionValue(8)};
      postSend(decoder.address, message);
    }
    else if(functionNumber <= 28)
    {
      switch(m_config.f9f28)
      {
        case LocoNetF9F28::IMMPacket:
        {
          const bool longAddress = decoder.protocol == DecoderProtocol::DCCLong;

          if(functionNumber <= 12)
          {
            if(longAddress)
            {
              postSend(
                LocoF9F12IMMLongAddress(
                  decoder.address,
                  decoder.getFunctionValue(9),
                  decoder.getFunctionValue(10),
                  decoder.getFunctionValue(11),
                  decoder.getFunctionValue(12)));
            }
            else
            {
              postSend(
                LocoF9F12IMMShortAddress(
                  decoder.address,
                  decoder.getFunctionValue(9),
                  decoder.getFunctionValue(10),
                  decoder.getFunctionValue(11),
                  decoder.getFunctionValue(12)));
            }
          }
          else if(functionNumber <= 20)
          {
            if(longAddress)
            {
              postSend(
                LocoF13F20IMMLongAddress(
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
              postSend(
                LocoF13F20IMMShortAddress(
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
          }
          else if(functionNumber <= 28)
          {
            if(longAddress)
            {
              postSend(
                LocoF21F28IMMLongAddress(
                  decoder.address,
                  decoder.getFunctionValue(21),
                  decoder.getFunctionValue(22),
                  decoder.getFunctionValue(23),
                  decoder.getFunctionValue(24),
                  decoder.getFunctionValue(25),
                  decoder.getFunctionValue(26),
                  decoder.getFunctionValue(27),
                  decoder.getFunctionValue(28)));
            }
            else
            {
              postSend(
                LocoF21F28IMMShortAddress(
                  decoder.address,
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
          break;
        }
        case LocoNetF9F28::UhlenbrockExtended:
          if(functionNumber <= 12)
          {
            LocoF9F12 message{
              decoder.getFunctionValue(9),
              decoder.getFunctionValue(10),
              decoder.getFunctionValue(11),
              decoder.getFunctionValue(12)};
            postSend(decoder.address, message);
          }
          else if(functionNumber <= 19)
          {
            LocoF13F19 message{
              decoder.getFunctionValue(13),
              decoder.getFunctionValue(14),
              decoder.getFunctionValue(15),
              decoder.getFunctionValue(16),
              decoder.getFunctionValue(17),
              decoder.getFunctionValue(18),
              decoder.getFunctionValue(19)};
            postSend(decoder.address, message);
          }
          else if(functionNumber == 20 || functionNumber == 28)
          {
            LocoF12F20F28 message{
              decoder.getFunctionValue(12),
              decoder.getFunctionValue(20),
              decoder.getFunctionValue(28)};
            postSend(decoder.address, message);
          }
          else if(functionNumber <= 27)
          {
            LocoF21F27 message{
              decoder.getFunctionValue(21),
              decoder.getFunctionValue(22),
              decoder.getFunctionValue(23),
              decoder.getFunctionValue(24),
              decoder.getFunctionValue(25),
              decoder.getFunctionValue(26),
              decoder.getFunctionValue(27)};
            postSend(decoder.address, message);
          }
          break;

        default:
          assert(false);
          break;
      }
    }
  }
}

bool Kernel::setOutput(OutputChannel channel, uint16_t address, OutputValue value)
{
  assert(isEventLoopThread());

  switch(channel)
  {
    case OutputChannel::Accessory:
      if(!inRange(address, accessoryOutputAddressMin, accessoryOutputAddressMax))
        return false;

      m_ioContext.post(
        [this, address, dir=std::get<OutputPairValue>(value) == OutputPairValue::Second]()
        {
          send(SwitchRequest(address, dir, true));
        });
      return true;

    case OutputChannel::DCCext:
      return
        inRange(address, DCC::Accessory::addressMin, DCC::Accessory::addressMax) &&
        inRange<int16_t>(std::get<int16_t>(value), std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max()) &&
        immPacket(DCC::SetAdvancedAccessoryValue(address, static_cast<uint8_t>(std::get<int16_t>(value))), 2);

    default: /*[[unlikely]]*/
      assert(false);
      break;
  }
  return false;
}

void Kernel::simulateInputChange(uint16_t address, SimulateInputAction action)
{
  assert(isEventLoopThread());
  assert(inRange(address, inputAddressMin, inputAddressMax));
  if(m_simulation)
    m_ioContext.post(
      [this, fullAddress=address - 1, action]()
      {
        switch(action)
        {
            case SimulateInputAction::SetFalse:
              if(m_inputValues[fullAddress] != TriState::False)
                receive(InputRep(fullAddress, false));
              break;

            case SimulateInputAction::SetTrue:
              if(m_inputValues[fullAddress] != TriState::True)
                receive(InputRep(fullAddress, true));
              break;

            case SimulateInputAction::Toggle:
              receive(InputRep(fullAddress, m_inputValues[fullAddress] != TriState::True));
              break;
        }
      });
}

void Kernel::lncvStart(uint16_t moduleId, uint16_t moduleAddress)
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this, moduleId, moduleAddress]()
    {
      if(m_lncvActive)
        return;

      m_lncvActive = true;
      m_lncvModuleId = moduleId;
      m_lncvModuleAddress = moduleAddress;
      send(Uhlenbrock::LNCVStart(m_lncvModuleId, m_lncvModuleAddress), HighPriority);
    });
}

void Kernel::lncvRead(uint16_t lncv)
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this, lncv]()
    {
      if(m_lncvActive)
        send(Uhlenbrock::LNCVRead(m_lncvModuleId, m_lncvModuleAddress, lncv), HighPriority);
    });
}

void Kernel::lncvWrite(uint16_t lncv, uint16_t value)
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this, lncv, value]()
    {
      if(m_lncvActive)
        send(Uhlenbrock::LNCVWrite(m_lncvModuleId, lncv, value), HighPriority);
    });
}

void Kernel::lncvStop()
{
  assert(isEventLoopThread());
  m_ioContext.post(
    [this]()
    {
      if(!m_lncvActive)
        return;

      send(Uhlenbrock::LNCVStop(m_lncvModuleId, m_lncvModuleAddress), HighPriority);
      m_lncvActive = false;
    });
}

void Kernel::setOnLNCVReadResponse(OnLNCVReadResponse callback)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_onLNCVReadResponse = std::move(callback);
}

Kernel::LocoSlot* Kernel::getLocoSlot(uint8_t slot, bool sendSlotDataRequestIfNew)
{
  assert(isKernelThread());

  if(!isLocoSlot(slot))
    return nullptr;

  auto it = m_slots.find(slot);
  if(it == m_slots.end())
  {
    if(sendSlotDataRequestIfNew)
      send(RequestSlotData(slot));
    it = m_slots.emplace(slot, LocoSlot()).first;
  }

  return &it->second;
}

Kernel::LocoSlot* Kernel::getLocoSlotByAddress(uint16_t address)
{
  assert(isKernelThread());

  auto it = std::find_if(m_slots.begin(), m_slots.end(),
    [address](auto& item)
    {
      assert(address != LocoSlot::invalidAddress);
      return item.second.address == address;
    });

  return it != m_slots.end() ? &it->second : nullptr;
}

void Kernel::clearLocoSlot(uint8_t slot)
{
  assert(isKernelThread());

  if(auto it = m_slots.find(slot); it != m_slots.end())
    m_slots.erase(it);

  if(auto it = std::find_if(m_addressToSlot.begin(), m_addressToSlot.end(), [slot](const auto& item){ return item.second == slot; }); it != m_addressToSlot.end())
    m_addressToSlot.erase(it);
}

std::shared_ptr<Decoder> Kernel::getDecoder(uint16_t address)
{
  assert(isEventLoopThread());
  return m_decoderController->getDecoder(DCC::getProtocol(address), address);
}

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(isEventLoopThread());
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

void Kernel::send(const Message& message, Priority priority)
{
  assert(isKernelThread());

  if(m_config.listenOnly)
    return; // drop it

  if(!m_sendQueue[priority].append(message))
  {
    // TODO: log message
    return;
  }

  if(!m_waitingForEcho && !m_waitingForResponse)
    sendNextMessage();
}

void Kernel::send(uint16_t address, Message& message, uint8_t& slot)
{
  assert(isKernelThread());

  if(auto addressToSlot = m_addressToSlot.find(address); addressToSlot != m_addressToSlot.end())
  {
    slot = addressToSlot->second;

    if(message.opCode == OPC_LOCO_SPD &&
        static_cast<LocoSpd&>(message).speed >= SPEED_MIN &&
        static_cast<LocoSpd&>(message).speed == m_slots[slot].speed)
      return; // same speed, don't send it (always send ESTOP and STOP)

    updateChecksum(message);
    send(message);
  }
  else // try get a slot
  {
    auto* ptr = reinterpret_cast<std::byte*>(&message);

    auto pendingSlotMessage = m_pendingSlotMessages.find(address);
    if(pendingSlotMessage == m_pendingSlotMessages.end())
    {
      m_pendingSlotMessages[address].assign(ptr, ptr + message.size());
      send(LocoAdr{address}, HighPriority);
    }
    else
      pendingSlotMessage->second.insert(pendingSlotMessage->second.end(), ptr, ptr + message.size());
  }
}

void Kernel::sendNextMessage()
{
  assert(isKernelThread());

  for(Priority priority = HighPriority; priority <= LowPriority; ++priority)
  {
    if(!m_sendQueue[priority].empty())
    {
      const Message& message = m_sendQueue[priority].front();

      if(m_config.debugLogRXTX)
        EventLoop::call([this, msg=toString(message)](){ Log::log(logId, LogMessage::D2001_TX_X, msg); });

      if(m_ioHandler->send(message))
      {
        m_sentMessagePriority = priority;

        m_waitingForEcho = true;
        m_waitingForEchoTimer.expires_after(boost::asio::chrono::milliseconds(m_config.echoTimeout));
        m_waitingForEchoTimer.async_wait(std::bind(&Kernel::waitingForEchoTimerExpired, this, std::placeholders::_1));

        m_waitingForResponse = hasResponse(message);
        if(m_waitingForResponse)
        {
          m_waitingForResponseTimer.expires_after(boost::asio::chrono::milliseconds(m_config.responseTimeout));
          m_waitingForResponseTimer.async_wait(std::bind(&Kernel::waitingForResponseTimerExpired, this, std::placeholders::_1));
        }
      }
      else
      {} // log message and go to error state
      return;
    }
  }
}

void Kernel::waitingForEchoTimerExpired(const boost::system::error_code& ec)
{
  assert(isKernelThread());

  if(ec)
    return;

  EventLoop::call(
    [this]()
    {
      Log::log(logId, LogMessage::W2018_TIMEOUT_NO_ECHO_WITHIN_X_MS, m_config.echoTimeout);
    });
}

void Kernel::waitingForResponseTimerExpired(const boost::system::error_code& ec)
{
  assert(isKernelThread());

  if(ec)
    return;

  if(m_lncvActive && Uhlenbrock::LNCVStart::check(lastSentMessage()))
  {
    EventLoop::call(
      [this, lncvStart=static_cast<const Uhlenbrock::LNCVStart&>(lastSentMessage())]()
      {
        Log::log(logId, LogMessage::N2002_NO_RESPONSE_FROM_LNCV_MODULE_X_WITH_ADDRESS_X, lncvStart.moduleId(), lncvStart.address());

        if(m_onLNCVReadResponse && m_lncvModuleId == lncvStart.moduleId())
          m_onLNCVReadResponse(false, lncvStart.address(), 0);
      });

    sendNextMessage();
  }
  else
  {
    EventLoop::call(
      [this]()
      {
        Log::log(logId, LogMessage::E2019_TIMEOUT_NO_RESPONSE_WITHIN_X_MS, m_config.responseTimeout);
        error();
      });
  }
}

void Kernel::setFastClockMaster(bool enable)
{
  assert(isKernelThread());
  if(enable)
  {
    const auto clock = m_fastClock.load();
    FastClockSlotWriteData fastClock;
    fastClock.clk_rate = clock.multiplier;
    fastClock.setHour(clock.hour);
    fastClock.setMinute(clock.minute);
    fastClock.setValid(true);
    fastClock.setId(idTraintastic);
    updateChecksum(fastClock);
    send(fastClock, HighPriority);
  }
  else
    send(FastClockSlotWriteData(), HighPriority);
}

void Kernel::disableClockEvents()
{
  m_clockChangeConnection.disconnect();
}

void Kernel::enableClockEvents()
{
  if(m_clockChangeConnection.connected() || !m_clock)
    return;

  m_clockChangeConnection = m_clock->onChange.connect(
    [this](Clock::ClockEvent event, uint8_t multiplier, Time time)
    {
      m_fastClock.store(FastClock{event == Clock::ClockEvent::Freeze ? multiplierFreeze : multiplier, time.hour(), time.minute()});
      if(event == Clock::ClockEvent::Freeze || event == Clock::ClockEvent::Resume)
      {
        m_ioContext.post(
          [this]()
          {
            setFastClockMaster(true);
          });
      }
    });
}

void Kernel::startFastClockSyncTimer()
{
  assert(isKernelThread());
  assert(m_config.fastClockSyncInterval > 0);
  m_fastClockSyncTimer.expires_after(boost::asio::chrono::seconds(m_config.fastClockSyncInterval));
  m_fastClockSyncTimer.async_wait(std::bind(&Kernel::fastClockSyncTimerExpired, this, std::placeholders::_1));
}

void Kernel::stopFastClockSyncTimer()
{
  assert(isKernelThread());
  m_fastClockSyncTimer.cancel();
}

void Kernel::fastClockSyncTimerExpired(const boost::system::error_code& ec)
{
  assert(isKernelThread());

  if(ec || !m_config.fastClockSyncEnabled || !m_fastClockSupported)
    return;

  send(RequestSlotData(SLOT_FAST_CLOCK));

  startFastClockSyncTimer();
}

template<uint8_t First, uint8_t Last, class T>
bool Kernel::updateFunctions(LocoSlot& slot, const T& message)
{
  assert(isKernelThread());

  bool changed = false;
  for(uint8_t i = First; i <= Last; ++i)
  {
    if(slot.functions[i] != message.f(i))
    {
      slot.functions[i] = toTriState(message.f(i));
      changed = true;
    }
  }

  EventLoop::call(
    [this, address=slot.address, message]()
    {
      if(auto decoder = getDecoder(address))
        for(uint8_t i = First; i <= Last; ++i)
          decoder->setFunctionValue(i, message.f(i));
    });

  return changed;
}

void Kernel::startPCAP(PCAPOutput pcapOutput)
{
  assert(isKernelThread());
  assert(!m_pcap);

  const uint32_t DLT_USER0 = 147; //! \todo change to LocoNet DLT once it is registered

  try
  {
    switch(pcapOutput)
    {
      case PCAPOutput::File:
      {
        const auto filename = m_debugDir / logId += dateTimeStr() += ".pcap";
        EventLoop::call(
          [this, filename]()
          {
            Log::log(logId, LogMessage::N2004_STARTING_PCAP_FILE_LOG_X, filename);
          });
        m_pcap = std::make_unique<PCAPFile>(filename, DLT_USER0);
        break;
      }
      case PCAPOutput::Pipe:
      {
        std::filesystem::path pipe;
#ifdef WIN32
        return; //! \todo Implement
#else // unix
        pipe = std::filesystem::temp_directory_path() / "traintastic-server" / logId;
#endif
        EventLoop::call(
          [this, pipe]()
          {
            Log::log(logId, LogMessage::N2005_STARTING_PCAP_LOG_PIPE_X, pipe);
          });
        m_pcap = std::make_unique<PCAPPipe>(std::move(pipe), DLT_USER0);
        break;
      }
    }
  }
  catch(const std::exception& e)
  {
    EventLoop::call(
      [this, what=std::string(e.what())]()
      {
        Log::log(logId, LogMessage::E2021_STARTING_PCAP_LOG_FAILED_X, what);
      });
  }
}


bool Kernel::SendQueue::append(const Message& message)
{
  const uint8_t messageSize = message.size();
  if(m_bytes + messageSize > threshold())
    return false;

  memcpy(m_front + m_bytes, &message, messageSize);
  m_bytes += messageSize;

  return true;
}

void Kernel::SendQueue::pop()
{
  const uint8_t messageSize = front().size();
  m_front += messageSize;
  m_bytes -= messageSize;

  if(static_cast<std::size_t>(m_front - m_buffer.data()) >= threshold())
  {
    memmove(m_buffer.data(), m_front, m_bytes);
    m_front = m_buffer.data();
  }
}

void Kernel::SendQueue::clear()
{
  m_bytes = 0;
}

}
