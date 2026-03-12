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

static std::string interpretTx1(uint8_t b)
{
  if(b == GlobalGo)   return "Global go";
  if(b == GlobalStop) return "Global stop";
  if(b == Extension::PollByte) return "Extension poll";
  if(b >= S88Base)
  return "S88 poll: " + std::to_string(b - S88Base) + " module(s)";
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
      return "Loco " + std::to_string(addr) +
                   ": direction toggle, F0=" + (f0 ? "on" : "off");
  return "Loco " + std::to_string(addr) +
               ": speed " + std::to_string(speed) +
               ", F0=" + (f0 ? "on" : "off");
  }

  // Accessory (32-34)
  if(cmd == AccessoryOff)   return "Accessory " + std::to_string(addr) + ": off";
  if(cmd == AccessoryGreen) return "Accessory " + std::to_string(addr) + ": green (straight)";
  if(cmd == AccessoryRed)   return "Accessory " + std::to_string(addr) + ": red (diverging)";

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

Kernel::Kernel(std::string logId_, const Config& config)
  : KernelBase{std::move(logId_)}
  , m_config{config}
  , m_strand{m_ioContext}
  , m_s88Timer{m_ioContext}
  , m_extensionTimer{m_ioContext}
{
}

Kernel::~Kernel() = default;

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  m_ioHandler = std::move(handler);
}

void Kernel::started()
{
  if(m_config.s88amount > 0)
  scheduleS88Poll();

  if(m_config.extensions)
  scheduleExtensionPoll();
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
      if(m_ioHandler)
  m_ioHandler->stop();
      m_ioContext.stop();
  });

  if(m_ioThread.joinable())
  m_ioThread.join();
}

void Kernel::sendGlobalGo()
{
  m_strand.post([this](){ sendWithRedundancy(GlobalGo); });
}

void Kernel::sendGlobalStop()
{
  m_strand.post([this](){ sendWithRedundancy(GlobalStop); });
}

void Kernel::setLocoSpeed(uint8_t address, uint8_t speed, bool f0)
{
  m_strand.post(
  [this, address, speed, f0]()
  {
      uint8_t cmd = speed & LocoSpeedMask;
      if(f0) cmd |= LocoF0Bit;
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
      if(f0) cmd |= LocoF0Bit;
      sendRaw(cmd, address);
  });
}

void Kernel::setLocoEmergencyStop(uint8_t address, bool f0)
{
  // Double direction-toggle: first stops, second restores direction.
  m_strand.post(
  [this, address, f0]()
  {
      uint8_t cmd = LocoDirToggle;
      if(f0) cmd |= LocoF0Bit;
      sendRaw(cmd, address);

      auto& t = m_redundancyTimers.emplace_back(m_ioContext);
      t.expires_after(50ms);
      t.async_wait(
    m_strand.wrap(
          [this, cmd, address](const boost::system::error_code& ec)
          {
      if(!ec && m_ioHandler)
              sendRaw(cmd, address);
          }));
  });
}

void Kernel::setLocoFunction(uint8_t address, uint8_t currentSpeed, bool f0)
{
  m_strand.post(
  [this, address, currentSpeed, f0]()
  {
      uint8_t cmd = currentSpeed & LocoSpeedMask;
      if(f0) cmd |= LocoF0Bit;
      sendWithRedundancy(cmd, address);
  });
}

void Kernel::setLocoFunctions1to4(uint8_t address, bool f1, bool f2, bool f3, bool f4)
{
  m_strand.post(
  [this, address, f1, f2, f3, f4]()
  {
      uint8_t cmd = FunctionBase;
      if(f1) cmd |= FunctionF1;
      if(f2) cmd |= FunctionF2;
      if(f3) cmd |= FunctionF3;
      if(f4) cmd |= FunctionF4;
      sendWithRedundancy(cmd, address);
  });
}

bool Kernel::setAccessory(uint32_t address, OutputValue value, unsigned int timeMs)
{
  if(address < 1 || address > 256)
  return false;

  uint8_t cmd = 0;
  std::visit(
  [&](auto&& v)
  {
      using T = std::decay_t<decltype(v)>;
      if constexpr(std::is_same_v<T, OutputPairValue>)
    cmd = (v == OutputPairValue::First) ? AccessoryRed : AccessoryGreen;
      else if constexpr(std::is_same_v<T, TriState>)
    cmd = (v == TriState::True) ? AccessoryRed : AccessoryGreen;
      else
    cmd = static_cast<uint8_t>(v);
  }, value);

  const uint8_t      addr       = static_cast<uint8_t>(address);
  const unsigned int redundancy = m_config.redundancy;

  m_strand.post(
  [this, cmd, addr, timeMs, redundancy]()
  {
      if(!m_ioHandler)
    return;

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
        sendRaw(cmd, addr);
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
        sendRaw(AccessoryOff, addr);
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
        sendRaw(AccessoryOff, addr);
      }));
      }
  });

  return true;
}

void Kernel::receive(uint8_t byte)
{
  if(m_config.debugLogRXTX)
  {
  char raw[4];
  std::snprintf(raw, sizeof(raw), "%02X", byte);

  // Build interpreted label based on current receive state
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
  const unsigned int byteIndex =
      (m_config.s88amount * 2) - m_s88Expect;
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
        s88Callback(contact, state);
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

void Kernel::sendRaw(uint8_t b1, uint8_t b2)
{
  if(m_ioHandler)
  {
  if(m_config.debugLogRXTX)
  {
      char raw[8];
      std::snprintf(raw, sizeof(raw), "%02X %02X", b1, b2);
      const std::string interp = interpretTx2(b1, b2);
      EventLoop::call([this, msg = std::string(raw) + "  [" + interp + "]"]()
              { Log::log(logId, LogMessage::D2001_TX_X, msg); });
  }
  m_ioHandler->send({b1, b2});
  }
}

void Kernel::sendRaw(uint8_t b)
{
  if(m_ioHandler)
  {
  if(m_config.debugLogRXTX)
  {
      char raw[4];
      std::snprintf(raw, sizeof(raw), "%02X", b);
      const std::string interp = interpretTx1(b);
      EventLoop::call([this, msg = std::string(raw) + "  [" + interp + "]"]()
              { Log::log(logId, LogMessage::D2001_TX_X, msg); });
  }
  m_ioHandler->send({b});
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
      sendRaw(b);
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
      sendRaw(b1, b2);
    }));
  }
}

void Kernel::scheduleS88Poll()
{
  m_s88Timer.expires_after(std::chrono::milliseconds(m_config.s88interval));
  m_s88Timer.async_wait(
  m_strand.wrap(
      [this](const boost::system::error_code& ec)
      {
    if(ec || !m_ioHandler)
          return;
    doS88Poll();
    scheduleS88Poll();
      }));
}

void Kernel::doS88Poll()
{
  const uint8_t cmd = S88Base + static_cast<uint8_t>(m_config.s88amount);
  sendRaw(cmd);
  m_s88State  = S88State::ReceivingData;
  m_s88Expect = m_config.s88amount * 2;
  m_s88Module = 0;
}

void Kernel::scheduleExtensionPoll()
{
  m_extensionTimer.expires_after(1s);
  m_extensionTimer.async_wait(
  m_strand.wrap(
      [this](const boost::system::error_code& ec)
      {
    if(ec || !m_ioHandler)
          return;
    doExtensionPoll();
    scheduleExtensionPoll();
      }));
}

void Kernel::doExtensionPoll()
{
  sendRaw(Extension::PollByte);
  sendRaw(Extension::PollByte);
  m_extState      = ExtState::WaitCount;
  m_extEventsLeft = 0;
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
              extensionGlobalCallback(power, run);
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
              extensionTurnoutCallback(address, green);
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
              extensionLocoCallback(address, speed, f0, forward);
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
              extensionFuncCallback(address, f1, f2, f3, f4);
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

} // namespace Marklin6050
