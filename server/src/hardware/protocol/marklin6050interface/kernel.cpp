/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Kamil Kasprzak
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
#include "protocol.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"

#include <chrono>
#include <cstdio>
#include <string>

using namespace std::chrono_literals;

namespace Marklin6050 {

// ---------------------------------------------------------------------------
// Command-queue overflow threshold: warn once the estimated time to drain the
// queue (pending frames * commandInterval) reaches this. 10 s = "too long".
// ---------------------------------------------------------------------------
static constexpr uint64_t kTxOverflowMs = 10000;

// ---------------------------------------------------------------------------
// Interpreted command logging helpers
// ---------------------------------------------------------------------------

static std::string interpretTx1(uint8_t b)
{
  if(b == GlobalGo)
  {
    return "Global go";
  }
  if(b == GlobalStop)
  {
    return "Global stop";
  }
  if(b == Extension::PollByte)
  {
    return "Extension poll";
  }
  if(b >= S88Base)
  {
    return "S88 poll: " + std::to_string(b - S88Base) + " module(s)";
  }
  return "?";
}

static std::string interpretTx2(uint8_t b1, uint8_t b2)
{
  const uint8_t cmd  = b1 & 0x7Fu; // strip any parity
  const uint8_t addr = b2;

  // Speed / F0 / direction toggle (bits 0-4)
  if(cmd <= 0x1Fu)
  {
    const uint8_t speed = cmd & LocoSpeedMask;
    const bool    f0    = cmd & LocoF0Bit;
    if(speed == LocoDirToggle)
    {
      return "Loco " + std::to_string(addr) +
             ": direction toggle, F0=" + (f0 ? "on" : "off");
    }
    return "Loco " + std::to_string(addr) +
           ": speed " + std::to_string(speed) +
           ", F0=" + (f0 ? "on" : "off");
  }

  // Accessory (32-34)
  if(cmd == AccessoryOff)
  {
    return "Accessory " + std::to_string(addr) + ": off";
  }
  if(cmd == AccessoryGreen)
  {
    return "Accessory " + std::to_string(addr) + ": green (straight)";
  }
  if(cmd == AccessoryRed)
  {
    return "Accessory " + std::to_string(addr) + ": red (diverging)";
  }

  // Functions F1-F4 (64-79)
  if(cmd >= FunctionBase && cmd < FunctionBase + 16u)
  {
    const uint8_t bits = cmd - FunctionBase;
    char buf[64];
    std::snprintf(buf, sizeof(buf), "Loco %u: F1=%u F2=%u F3=%u F4=%u",
                  addr,
                  (bits & FunctionF1) ? 1u : 0u,
                  (bits & FunctionF2) ? 1u : 0u,
                  (bits & FunctionF3) ? 1u : 0u,
                  (bits & FunctionF4) ? 1u : 0u);
    return buf;
  }

  char buf[16];
  std::snprintf(buf, sizeof(buf), "%02X %02X", b1, b2);
  return buf;
}


// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Kernel::Kernel(std::string logId_, const Config& config)
  : KernelBase{std::move(logId_)}
  , m_config{config}
  , m_strand{m_ioContext}
  , m_s88Timer{m_ioContext}
  , m_extensionTimer{m_ioContext}
  , m_txTimer{m_ioContext}
  , m_ctsMonitorTimer{m_ioContext}
{
}

Kernel::~Kernel() = default;

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  m_ioHandler = std::move(handler);
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void Kernel::started()
{
  if(m_config.s88amount > 0)
  {
    scheduleS88Poll();
  }

  if(m_config.extensions)
  {
    scheduleExtensionPoll();
  }

  if(m_config.crashDetection)
  {
    scheduleCtsMonitor();
  }
}

void Kernel::start()
{
  assert(m_ioHandler);
  m_ioThread = std::thread([this](){ m_ioContext.run(); });
  m_strand.post([this](){ m_ioHandler->start(); });
}

void Kernel::stop()
{
  m_strand.post(
    [this]()
    {
      m_s88Timer.cancel();
      m_extensionTimer.cancel();
      m_redundancyTimers.clear();
      m_txTimer.cancel();
      m_ctsMonitorTimer.cancel();
      m_txTimerArmed = false;
      m_txQueue.clear();
      if(m_ioHandler)
      {
        m_ioHandler->stop();
      }
      m_ioContext.stop();
    });

  if(m_ioThread.joinable())
  {
    m_ioThread.join();
  }
}

// ---------------------------------------------------------------------------
// Public command API  (posts onto the strand)
// ---------------------------------------------------------------------------

void Kernel::sendGlobalGo()
{
  m_strand.post([this](){ sendWithRedundancy(GlobalGo); });
}

void Kernel::sendGlobalStop()
{
  // Stop bypasses the command queue so it is never delayed by backlog.
  m_strand.post([this](){ sendImmediateWithRedundancy({GlobalStop}); });
}

void Kernel::setLocoSpeed(uint8_t address, uint8_t speed, bool f0)
{
  m_strand.post(
    [this, address, speed, f0]()
    {
      uint8_t cmd = speed & LocoSpeedMask;
      if(f0)
      {
        cmd |= LocoF0Bit;
      }
      sendWithRedundancy(cmd, address);
    });
}

void Kernel::setLocoDirection(uint8_t address, bool f0)
{
  // No redundancy – toggling twice cancels out.
  m_strand.post(
    [this, address, f0]()
    {
      uint8_t cmd = LocoDirToggle;
      if(f0)
      {
        cmd |= LocoF0Bit;
      }
      sendRaw(cmd, address);
    });
}

void Kernel::setLocoEmergencyStop(uint8_t address, bool f0)
{
  // Double direction-toggle: first stops, second restores direction.
  // Emergency stop bypasses the command queue (sent immediately).
  m_strand.post(
    [this, address, f0]()
    {
      uint8_t cmd = LocoDirToggle;
      if(f0)
      {
        cmd |= LocoF0Bit;
      }
      writeFrameNow({cmd, address});

      auto& t = m_redundancyTimers.emplace_back(m_ioContext);
      t.expires_after(50ms);
      t.async_wait(
        m_strand.wrap(
          [this, cmd, address](const boost::system::error_code& ec)
          {
            if(!ec && m_ioHandler)
            {
              writeFrameNow({cmd, address});
            }
          }));
    });
}

void Kernel::setLocoFunction(uint8_t address, uint8_t currentSpeed, bool f0)
{
  m_strand.post(
    [this, address, currentSpeed, f0]()
    {
      uint8_t cmd = currentSpeed & LocoSpeedMask;
      if(f0)
      {
        cmd |= LocoF0Bit;
      }
      sendWithRedundancy(cmd, address);
    });
}

void Kernel::setLocoFunctions1to4(uint8_t address, bool f1, bool f2, bool f3, bool f4)
{
  m_strand.post(
    [this, address, f1, f2, f3, f4]()
    {
      uint8_t cmd = FunctionBase;
      if(f1) { cmd |= FunctionF1; }
      if(f2) { cmd |= FunctionF2; }
      if(f3) { cmd |= FunctionF3; }
      if(f4) { cmd |= FunctionF4; }
      sendWithRedundancy(cmd, address);
    });
}

bool Kernel::setAccessory(uint32_t address, OutputValue value, unsigned int timeMs)
{
  if(address < 1 || address > 256)
  {
    return false;
  }

  uint8_t cmd = 0;
  std::visit(
    [&](auto&& v)
    {
      using T = std::decay_t<decltype(v)>;
      if constexpr(std::is_same_v<T, OutputPairValue>)
      {
        cmd = (v == OutputPairValue::First) ? AccessoryRed : AccessoryGreen;
      }
      else if constexpr(std::is_same_v<T, TriState>)
      {
        cmd = (v == TriState::True) ? AccessoryRed : AccessoryGreen;
      }
      else
      {
        cmd = static_cast<uint8_t>(v);
      }
    }, value);

  const uint8_t      addr       = static_cast<uint8_t>(address);
  const unsigned int redundancy = m_config.redundancy;

  m_strand.post(
    [this, cmd, addr, timeMs, redundancy]()
    {
      if(!m_ioHandler)
      {
        return;
      }

      sendRaw(cmd, addr);

      unsigned int offset = 0;

      // Redundant activations
      for(unsigned int i = 0; i < redundancy; ++i)
      {
        offset += 50;
        auto& t = m_redundancyTimers.emplace_back(m_ioContext);
        t.expires_after(std::chrono::milliseconds(offset));
        t.async_wait(
          m_strand.wrap(
            [this, cmd, addr](const boost::system::error_code& ec)
            {
              if(!ec && m_ioHandler)
              {
                sendRaw(cmd, addr);
              }
            }));
      }

      // Solenoid off
      offset += timeMs;
      {
        auto& t = m_redundancyTimers.emplace_back(m_ioContext);
        t.expires_after(std::chrono::milliseconds(offset));
        t.async_wait(
          m_strand.wrap(
            [this, addr](const boost::system::error_code& ec)
            {
              if(!ec && m_ioHandler)
              {
                sendRaw(AccessoryOff, addr);
              }
            }));
      }

      // Redundant deactivations
      for(unsigned int i = 0; i < redundancy; ++i)
      {
        offset += 50;
        auto& t = m_redundancyTimers.emplace_back(m_ioContext);
        t.expires_after(std::chrono::milliseconds(offset));
        t.async_wait(
          m_strand.wrap(
            [this, addr](const boost::system::error_code& ec)
            {
              if(!ec && m_ioHandler)
              {
                sendRaw(AccessoryOff, addr);
              }
            }));
      }
    });

  return true;
}

// ---------------------------------------------------------------------------
// IOHandler callbacks  (arrive on m_strand)
// ---------------------------------------------------------------------------

void Kernel::receive(uint8_t byte)
{
  if(m_config.debugLogRXTX)
  {
    char raw[4];
    std::snprintf(raw, sizeof(raw), "%02X", byte);

    std::string interp;
    if(m_s88State == S88State::ReceivingData)
    {
      const unsigned int byteIndex = (m_config.s88amount * 2) - m_s88Expect;
      const bool isHighByte = (byteIndex % 2 == 0);
      char ibuf[48];
      std::snprintf(ibuf, sizeof(ibuf), "S88 module %u %s byte",
                    m_s88Module + 1,
                    isHighByte ? "high" : "low");
      interp = ibuf;
    }
    else if(m_config.extensions && m_extState != ExtState::Idle)
    {
      interp = "ext-event byte";
    }
    else
    {
      interp = "unexpected";
    }

    EventLoop::call([this, msg = std::string(raw) + "  [" + interp + "]"]()
      { Log::log(logId, LogMessage::D2002_RX_X, msg); });
  }

  // S88 receive state machine
  if(m_s88State == S88State::ReceivingData)
  {
    const unsigned int byteIndex = (m_config.s88amount * 2) - m_s88Expect;
    --m_s88Expect;

    const bool isHighByte = (byteIndex % 2 == 0);
    if(isHighByte)
    {
      m_s88High = byte;
    }
    else
    {
      const uint16_t bits =
        (static_cast<uint16_t>(m_s88High) << 8) |
        static_cast<uint16_t>(byte);
      const unsigned int moduleIdx = m_s88Module++;

      if(s88Callback)
      {
        for(int bit = 0; bit < 16; ++bit)
        {
          const bool     state   = (bits >> bit) & 1u;
          const uint32_t contact = moduleIdx * 16 + (bit + 1);

          EventLoop::call(
            [this, contact, state]()
            {
              if(s88Callback)
              {
                s88Callback(contact, state);
              }
            });
        }
      }
    }

    if(m_s88Expect == 0)
    {
      m_s88State  = S88State::Idle;
      m_s88Module = 0;
    }
    return;
  }

  // Extension receive state machine
  if(m_config.extensions && m_extState != ExtState::Idle)
  {
    processExtensionByte(byte);
  }
}

void Kernel::readError(const boost::system::error_code& ec)
{
  EventLoop::call(
    [this, ec]()
    {
      Log::log(logId, LogMessage::E2002_SERIAL_READ_FAILED_X, ec);
    });
}

void Kernel::writeError(const boost::system::error_code& ec)
{
  EventLoop::call(
    [this, ec]()
    {
      Log::log(logId, LogMessage::E2001_SERIAL_WRITE_FAILED_X, ec);
    });
}

// ---------------------------------------------------------------------------
// Internal send helpers  (must be on m_strand)
// ---------------------------------------------------------------------------

void Kernel::sendRaw(uint8_t b1, uint8_t b2)
{
  if(!m_ioHandler)
  {
    return;
  }
  if(m_config.commandQueue)
  {
    enqueueTx({b1, b2}, nullptr);
  }
  else
  {
    writeFrameNow({b1, b2});
  }
}

void Kernel::sendRaw(uint8_t b)
{
  if(!m_ioHandler)
  {
    return;
  }
  if(m_config.commandQueue)
  {
    enqueueTx({b}, nullptr);
  }
  else
  {
    writeFrameNow({b});
  }
}

// Write a 1- or 2-byte frame to the wire immediately (the unpaced path, and the
// path the drain timer uses). Must run on m_strand.
void Kernel::writeFrameNow(const std::vector<uint8_t>& frame)
{
  if(!m_ioHandler || frame.empty())
  {
    return;
  }

  if(m_config.debugLogRXTX)
  {
    char raw[8];
    std::string interp;
    if(frame.size() == 1)
    {
      std::snprintf(raw, sizeof(raw), "%02X", frame[0]);
      interp = interpretTx1(frame[0]);
    }
    else
    {
      std::snprintf(raw, sizeof(raw), "%02X %02X", frame[0], frame[1]);
      interp = interpretTx2(frame[0], frame[1]);
    }
    EventLoop::call([this, msg = std::string(raw) + "  [" + interp + "]"]()
      { Log::log(logId, LogMessage::D2001_TX_X, msg); });
  }

  if(frame.size() == 1)
  {
    m_ioHandler->send({frame[0]});
  }
  else
  {
    m_ioHandler->send({frame[0], frame[1]});
  }
}

void Kernel::enqueueTx(std::vector<uint8_t> frame, std::function<void()> onSent)
{
  // Bound the queue: once the backlog reaches the overflow threshold, drop new
  // frames so memory cannot grow without bound (e.g. if S88 is polled faster
  // than commandInterval lets it drain). The overflow response has already run.
  if(static_cast<uint64_t>(m_txQueue.size()) * m_config.commandInterval >= kTxOverflowMs)
  {
    checkTxOverflow();
    return;
  }

  // Single FIFO: S88/extension polls and loco/accessory commands share one
  // line and are sent in enqueue order (the hardware makes no distinction).
  m_txQueue.push_back(TxItem{std::move(frame), std::move(onSent)});
  checkTxOverflow();
  armTxTimer();
}

void Kernel::armTxTimer()
{
  if(m_txTimerArmed || !m_ioHandler)
  {
    return;
  }
  if(m_txQueue.empty())
  {
    return;
  }

  m_txTimerArmed = true;
  m_txTimer.expires_after(std::chrono::milliseconds(m_config.commandInterval));
  m_txTimer.async_wait(
    m_strand.wrap(
      [this](const boost::system::error_code& ec)
      {
        m_txTimerArmed = false;
        if(ec || !m_ioHandler) // cancelled (stop) or no handler
        {
          return;
        }
        drainTx();
      }));
}

void Kernel::drainTx()
{
  // wait-for-CTS: hold the whole queue until the station asserts CTS (ready).
  if(m_config.waitForCts && m_ioHandler && !m_ioHandler->getCTS())
  {
    armTxTimer(); // retry after commandInterval without consuming a frame
    return;
  }

  if(m_txQueue.empty())
  {
    return;
  }
  TxItem item = std::move(m_txQueue.front());
  m_txQueue.pop_front();

  writeFrameNow(item.data);
  if(item.onSent)
  {
    item.onSent();
  }

  checkTxOverflow();
  armTxTimer(); // keep draining while frames remain
}

void Kernel::checkTxOverflow()
{
  const std::size_t pending = m_txQueue.size();
  const uint64_t backlogMs =
    static_cast<uint64_t>(pending) * m_config.commandInterval;

  if(backlogMs >= kTxOverflowMs)
  {
    if(!m_txOverflowFired)
    {
      m_txOverflowFired = true;
      EventLoop::call(
        [this, pending]()
        {
          // Always log the critical message; stop the world only when warnings
          // are not being ignored.
          Log::log(logId, LogMessage::C2006_COMMAND_QUEUE_OVERFLOWING_X,
                   static_cast<uint32_t>(pending));
          if(!m_config.ignoreWarnings && stopWorldCallback)
          {
            stopWorldCallback();
          }
        });
    }
  }
  else if(backlogMs < kTxOverflowMs / 2)
  {
    m_txOverflowFired = false; // hysteresis: allow a fresh trigger later
  }
}

void Kernel::sendWithRedundancy(uint8_t b)
{
  sendRaw(b);
  for(unsigned int i = 0; i < m_config.redundancy; ++i)
  {
    auto& t = m_redundancyTimers.emplace_back(m_ioContext);
    t.expires_after(std::chrono::milliseconds(50u * (i + 1)));
    t.async_wait(
      m_strand.wrap(
        [this, b](const boost::system::error_code& ec)
        {
          if(!ec && m_ioHandler)
          {
            sendRaw(b);
          }
        }));
  }
}

void Kernel::sendWithRedundancy(uint8_t b1, uint8_t b2)
{
  sendRaw(b1, b2);
  for(unsigned int i = 0; i < m_config.redundancy; ++i)
  {
    auto& t = m_redundancyTimers.emplace_back(m_ioContext);
    t.expires_after(std::chrono::milliseconds(50u * (i + 1)));
    t.async_wait(
      m_strand.wrap(
        [this, b1, b2](const boost::system::error_code& ec)
        {
          if(!ec && m_ioHandler)
          {
            sendRaw(b1, b2);
          }
        }));
  }
}

// Like sendWithRedundancy but writes straight to the wire, bypassing the
// pacing queue — used for stop commands so they are never delayed by backlog.
void Kernel::sendImmediateWithRedundancy(std::vector<uint8_t> frame)
{
  writeFrameNow(frame);
  for(unsigned int i = 0; i < m_config.redundancy; ++i)
  {
    auto& t = m_redundancyTimers.emplace_back(m_ioContext);
    t.expires_after(std::chrono::milliseconds(50u * (i + 1)));
    t.async_wait(
      m_strand.wrap(
        [this, frame](const boost::system::error_code& ec)
        {
          if(!ec && m_ioHandler)
          {
            writeFrameNow(frame);
          }
        }));
  }
}

// ---------------------------------------------------------------------------
// S88 polling
// ---------------------------------------------------------------------------

void Kernel::scheduleS88Poll()
{
  m_s88Timer.expires_after(std::chrono::milliseconds(m_config.s88interval));
  m_s88Timer.async_wait(
    m_strand.wrap(
      [this](const boost::system::error_code& ec)
      {
        if(ec || !m_ioHandler)
        {
          return;
        }
        doS88Poll();
        scheduleS88Poll();
      }));
}

void Kernel::doS88Poll()
{
  const uint8_t cmd = S88Base + static_cast<uint8_t>(m_config.s88amount);

  // Enter the "receiving module data" state only once the poll byte has
  // actually gone out (it may wait its turn in the single FIFO behind
  // commands). The paced drain sends one frame per interval, so only one poll
  // is ever awaiting its reply at a time.
  auto onSent = [this]()
  {
    m_s88State  = S88State::ReceivingData;
    m_s88Expect = m_config.s88amount * 2;
    m_s88Module = 0;
  };

  if(m_config.commandQueue)
  {
    enqueueTx({cmd}, onSent);
  }
  else
  {
    sendRaw(cmd);
    onSent();
  }
}

// ---------------------------------------------------------------------------
// Extension polling
// ---------------------------------------------------------------------------

void Kernel::scheduleExtensionPoll()
{
  m_extensionTimer.expires_after(1s);
  m_extensionTimer.async_wait(
    m_strand.wrap(
      [this](const boost::system::error_code& ec)
      {
        if(ec || !m_ioHandler)
        {
          return;
        }
        doExtensionPoll();
        scheduleExtensionPoll();
      }));
}

void Kernel::doExtensionPoll()
{
  // Enter the extension-receive state only once the poll bytes are really sent.
  auto onSent = [this]()
  {
    m_extState      = ExtState::WaitCount;
    m_extEventsLeft = 0;
  };

  if(m_config.commandQueue)
  {
    // Both poll bytes go out as one frame so they stay adjacent on the wire.
    enqueueTx({Extension::PollByte, Extension::PollByte}, onSent);
  }
  else
  {
    sendRaw(Extension::PollByte);
    sendRaw(Extension::PollByte);
    onSent();
  }
}

void Kernel::processExtensionByte(uint8_t byte)
{
  switch(m_extState)
  {
    case ExtState::WaitCount:
      m_extEventsLeft = byte;
      m_extState = (byte > 0) ? ExtState::WaitType : ExtState::Idle;
      break;

    case ExtState::WaitType:
      switch(byte)
      {
        case Extension::EventGlobal:    m_extState = ExtState::GlobalData;    break;
        case Extension::EventTurnout:   m_extState = ExtState::TurnoutAddr;   break;
        case Extension::EventLocoState: m_extState = ExtState::LocoStateAddr; break;
        case Extension::EventLocoFunc:  m_extState = ExtState::LocoFuncAddr;  break;
        default:
          m_extState = ExtState::Idle; // unknown – bail
          break;
      }
      break;

    case ExtState::GlobalData:
    {
      const bool power = byte & Extension::GlobalPowerBit;
      const bool run   = byte & Extension::GlobalRunBit;
      if(extensionGlobalCallback)
      {
        EventLoop::call(
          [this, power, run]()
          {
            if(extensionGlobalCallback)
            {
              extensionGlobalCallback(power, run);
            }
          });
      }
      advanceExtensionEvent();
      break;
    }

    case ExtState::TurnoutAddr:
      m_extTmpAddr = byte;
      m_extState   = ExtState::TurnoutState;
      break;

    case ExtState::TurnoutState:
    {
      const uint32_t address =
        (m_extTmpAddr == 0) ? 256u : static_cast<uint32_t>(m_extTmpAddr);
      const bool green = (byte != 0);
      if(extensionTurnoutCallback)
      {
        EventLoop::call(
          [this, address, green]()
          {
            if(extensionTurnoutCallback)
            {
              extensionTurnoutCallback(address, green);
            }
          });
      }
      advanceExtensionEvent();
      break;
    }

    case ExtState::LocoStateAddr:
      m_extTmpAddr = byte;
      m_extState   = ExtState::LocoStateData;
      break;

    case ExtState::LocoStateData:
    {
      const uint8_t address = m_extTmpAddr;
      const uint8_t speed   = byte & Extension::LocoSpeedBits;
      const bool    f0      = byte & Extension::LocoF0Bit_Ext;
      const bool    forward = !(byte & Extension::LocoDirBit);
      if(extensionLocoCallback)
      {
        EventLoop::call(
          [this, address, speed, f0, forward]()
          {
            if(extensionLocoCallback)
            {
              extensionLocoCallback(address, speed, f0, forward);
            }
          });
      }
      advanceExtensionEvent();
      break;
    }

    case ExtState::LocoFuncAddr:
      m_extTmpAddr = byte;
      m_extState   = ExtState::LocoFuncData;
      break;

    case ExtState::LocoFuncData:
    {
      const uint8_t address = m_extTmpAddr;
      const bool f1 = byte & Extension::LocoF1Bit;
      const bool f2 = byte & Extension::LocoF2Bit;
      const bool f3 = byte & Extension::LocoF3Bit;
      const bool f4 = byte & Extension::LocoF4Bit;
      if(extensionFuncCallback)
      {
        EventLoop::call(
          [this, address, f1, f2, f3, f4]()
          {
            if(extensionFuncCallback)
            {
              extensionFuncCallback(address, f1, f2, f3, f4);
            }
          });
      }
      advanceExtensionEvent();
      break;
    }

    default:
      m_extState = ExtState::Idle;
      break;
  }
}

void Kernel::advanceExtensionEvent()
{
  m_extState = (--m_extEventsLeft > 0) ? ExtState::WaitType : ExtState::Idle;
}

// ---------------------------------------------------------------------------
// Crash detection — poll the serial CTS line once per second. If it stays low
// for more than 10 s the command station has probably crashed or been
// disconnected: on the EventLoop thread, log a critical message and invoke
// stopWorldCallback to power the world off.
// ---------------------------------------------------------------------------

void Kernel::scheduleCtsMonitor()
{
  m_ctsMonitorTimer.expires_after(std::chrono::seconds(1));
  m_ctsMonitorTimer.async_wait(
    m_strand.wrap(
      [this](const boost::system::error_code& ec)
      {
        if(ec || !m_ioHandler) // cancelled (stop) or no handler
        {
          return;
        }

        if(m_ioHandler->getCTS())
        {
          m_ctsLowSince.reset();
          m_crashFired = false; // recovered — allow a future detection
        }
        else
        {
          const auto now = std::chrono::steady_clock::now();
          if(!m_ctsLowSince)
          {
            m_ctsLowSince = now;
          }
          else if(!m_crashFired && (now - *m_ctsLowSince) >= std::chrono::seconds(10))
          {
            m_crashFired = true;
            EventLoop::call(
              [this]()
              {
                // Always log the crash; stop the world only when warnings are
                // not being ignored.
                Log::log(logId, LogMessage::C2007_COMMAND_STATION_CRASH_DETECTED);
                if(!m_config.ignoreWarnings && stopWorldCallback)
                {
                  stopWorldCallback();
                }
              });
          }
        }

        scheduleCtsMonitor(); // keep monitoring
      }));
}

} // namespace Marklin6050
