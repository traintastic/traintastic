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

namespace Marklin6023 {

// ---------------------------------------------------------------------------
// S88 response timeout: if the device doesn't reply within this period,
// we skip the current contact and continue — prevents the cycle hanging.
// ---------------------------------------------------------------------------
static constexpr auto kS88ResponseTimeout = std::chrono::milliseconds(1000);

// ---------------------------------------------------------------------------
// Command-queue overflow threshold: warn once the estimated time to drain the
// queue (pending frames * commandInterval) reaches this. 10 s = "too long".
// ---------------------------------------------------------------------------
static constexpr uint64_t kTxOverflowMs = 10000;

// ---------------------------------------------------------------------------
// Interpreted command logging helpers
// ---------------------------------------------------------------------------

static std::string interpretTx(const std::string& cmd)
{
  if(cmd.empty())
  {
    return cmd;
  }

  // "G" – Global Go
  if(cmd == "G")
  {
    return "Global go";
  }

  // "S" – Global Stop
  if(cmd == "S")
  {
    return "Global stop";
  }

  // "L <addr> S <spd> F <f0>" – loco speed + F0
  // "L <addr> D"              – loco direction toggle
  if(cmd.size() >= 2 && cmd[0] == 'L' && cmd[1] == ' ')
  {
    const std::size_t addrStart = 2;
    const std::size_t spaceAfterAddr = cmd.find(' ', addrStart);
    if(spaceAfterAddr == std::string::npos)
    {
      return cmd;
    }
    const std::string addr = cmd.substr(addrStart, spaceAfterAddr - addrStart);
    const std::string rest = cmd.substr(spaceAfterAddr + 1);

    if(!rest.empty() && rest[0] == 'D')
    {
      return "Loco " + addr + ": direction toggle";
    }

    unsigned int spd = 0, f0 = 0;
    if(std::sscanf(rest.c_str(), "S %u F %u", &spd, &f0) == 2)
    {
      return "Loco " + addr + ": speed " + std::to_string(spd) +
             ", F0=" + (f0 ? "on" : "off");
    }

    return cmd;
  }

  // "M <addr> R/G" – accessory
  if(cmd.size() >= 2 && cmd[0] == 'M' && cmd[1] == ' ')
  {
    const std::size_t addrStart = 2;
    const std::size_t spaceAfterAddr = cmd.find(' ', addrStart);
    if(spaceAfterAddr != std::string::npos && spaceAfterAddr + 1 < cmd.size())
    {
      const std::string addr = cmd.substr(addrStart, spaceAfterAddr - addrStart);
      const char dir = cmd[spaceAfterAddr + 1];
      return std::string("Accessory ") + addr +
             (dir == 'R' ? ": red (diverging)" : ": green (straight)");
    }
  }

  // "C <n>" – S88 contact query
  if(cmd.size() >= 3 && cmd[0] == 'C' && cmd[1] == ' ')
  {
    const std::string contact = cmd.substr(2);
    return "S88 query contact " + contact;
  }

  return cmd;
}

static std::string interpretRx(const std::string& line, uint32_t queriedContact)
{
  // S88 response is a single digit "0" or "1"
  if(line == "0" || line == "1")
  {
    const std::string state = (line == "1") ? "occupied" : "clear";
    return "S88 contact " + std::to_string(queriedContact) + ": " + state;
  }
  return line;
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Kernel::Kernel(std::string logId_, const Config& config)
  : KernelBase{std::move(logId_)}
  , m_config{config}
  , m_strand{m_ioContext}
  , m_s88Timer{m_ioContext}
  , m_s88ResponseTimer{m_ioContext}
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
    startS88Cycle();
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
      m_s88ResponseTimer.cancel();
      m_redundancyTimers.clear();
      m_txTimer.cancel();
      m_txTimerArmed = false;
      m_txQueue.clear();
      m_ctsMonitorTimer.cancel();
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
// Public command API
// ---------------------------------------------------------------------------

void Kernel::sendGlobalGo()
{
  m_strand.post([this](){ sendCmdWithRedundancy("G"); });
}

void Kernel::sendGlobalStop()
{
  // Stop bypasses the command queue so it is never delayed by backlog.
  m_strand.post([this](){ sendImmediateWithRedundancy("S"); });
}

void Kernel::setLocoSpeed(uint8_t address, uint8_t speed, bool f0)
{
  m_strand.post(
    [this, address, speed, f0]()
    {
      sendCmdWithRedundancy(
        "L " + std::to_string(address) +
        " S " + std::to_string(speed & 0x0Fu) +
        " F " + (f0 ? "1" : "0"));
    });
}

void Kernel::setLocoDirection(uint8_t address, bool /*f0*/)
{
  m_strand.post(
    [this, address]()
    {
      sendCmd("L " + std::to_string(address) + " D");
    });
}

void Kernel::setLocoEmergencyStop(uint8_t address, bool /*f0*/)
{
  m_strand.post(
    [this, address]()
    {
      // Loco emergency stop bypasses the command queue (sent immediately).
      const std::string cmd = "L " + std::to_string(address) + " D";
      writeCmdNow(cmd);

      auto& t = m_redundancyTimers.emplace_back(m_ioContext);
      t.expires_after(50ms);
      t.async_wait(
        m_strand.wrap(
          [this, cmd](const boost::system::error_code& ec)
          {
            if(!ec && m_ioHandler)
            {
              writeCmdNow(cmd);
            }
          }));
    });
}

void Kernel::setLocoFunction(uint8_t address, uint8_t currentSpeed, bool f0)
{
  setLocoSpeed(address, currentSpeed, f0);
}

bool Kernel::setAccessory(uint32_t address, OutputValue value)
{
  if(address < 1 || address > 256)
  {
    return false;
  }

  char dir = 'G';
  std::visit(
    [&](auto&& v)
    {
      using T = std::decay_t<decltype(v)>;
      if constexpr(std::is_same_v<T, OutputPairValue>)
      {
        dir = (v == OutputPairValue::First) ? 'R' : 'G';
      }
      else if constexpr(std::is_same_v<T, TriState>)
      {
        dir = (v == TriState::True) ? 'R' : 'G';
      }
    }, value);

  m_strand.post(
    [this, address, dir]()
    {
      sendCmdWithRedundancy("M " + std::to_string(address) + " " + dir);
    });

  return true;
}

// ---------------------------------------------------------------------------
// IOHandler callbacks  (arrive on m_strand)
// ---------------------------------------------------------------------------

void Kernel::receiveLine(std::string line)
{
  if(m_config.debugLogRXTX)
  {
    const std::string interp = m_s88WaitingReply
      ? interpretRx(line, m_s88LastQueried)
      : line;
    EventLoop::call(
      [this, raw = line, interp]()
      {
        Log::log(logId, LogMessage::D2002_RX_X,
                 raw + "  [" + interp + "]");
      });
  }

  if(m_s88WaitingReply)
  {
    onS88Response(line);
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

void Kernel::sendCmd(std::string cmd)
{
  if(!m_ioHandler)
  {
    return;
  }

  if(m_config.commandQueue)
  {
    enqueueCmd(std::move(cmd), nullptr);
  }
  else
  {
    writeCmdNow(cmd);
  }
}

// Write a command to the wire immediately (the unpaced path, and the path the
// drain timer uses). Must run on m_strand.
void Kernel::writeCmdNow(const std::string& cmd)
{
  if(!m_ioHandler)
  {
    return;
  }

  if(m_config.debugLogRXTX)
  {
    const std::string interp = interpretTx(cmd);
    EventLoop::call(
      [this, raw = cmd, interp]()
      {
        Log::log(logId, LogMessage::D2001_TX_X,
                 raw + "  [" + interp + "]");
      });
  }

  m_ioHandler->sendString(cmd + CR);
}

void Kernel::enqueueCmd(std::string cmd, std::function<void()> onSent)
{
  // Bound the queue: once the backlog reaches the overflow threshold, drop new
  // frames so memory cannot grow without bound (e.g. if S88 is polled faster
  // than commandInterval lets it drain). The overflow response has already run.
  if(static_cast<uint64_t>(m_txQueue.size()) * m_config.commandInterval >= kTxOverflowMs)
  {
    checkTxOverflow();
    return;
  }

  // Single FIFO: S88 queries and loco/accessory commands share one line and
  // are sent in enqueue order (the hardware makes no distinction).
  m_txQueue.push_back(TxItem{std::move(cmd), std::move(onSent)});
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

  writeCmdNow(item.data);
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

void Kernel::sendCmdWithRedundancy(std::string cmd)
{
  sendCmd(cmd);
  for(unsigned int i = 0; i < m_config.redundancy; ++i)
  {
    auto& t = m_redundancyTimers.emplace_back(m_ioContext);
    t.expires_after(std::chrono::milliseconds(50u * (i + 1)));
    t.async_wait(
      m_strand.wrap(
        [this, cmd](const boost::system::error_code& ec)
        {
          if(!ec && m_ioHandler)
          {
            sendCmd(cmd);
          }
        }));
  }
}

// Like sendCmdWithRedundancy but writes straight to the wire, bypassing the
// pacing queue — used for stop commands so they are never delayed by backlog.
void Kernel::sendImmediateWithRedundancy(std::string cmd)
{
  writeCmdNow(cmd);
  for(unsigned int i = 0; i < m_config.redundancy; ++i)
  {
    auto& t = m_redundancyTimers.emplace_back(m_ioContext);
    t.expires_after(std::chrono::milliseconds(50u * (i + 1)));
    t.async_wait(
      m_strand.wrap(
        [this, cmd](const boost::system::error_code& ec)
        {
          if(!ec && m_ioHandler)
          {
            writeCmdNow(cmd);
          }
        }));
  }
}

// ---------------------------------------------------------------------------
// S88 polling
// ---------------------------------------------------------------------------

void Kernel::startS88Cycle()
{
  m_s88NextContact  = 1;
  m_s88WaitingReply = false;
  m_s88LastQueried  = 0;

  m_s88Timer.expires_after(std::chrono::milliseconds(m_config.s88interval));
  m_s88Timer.async_wait(
    m_strand.wrap(
      [this](const boost::system::error_code& ec)
      {
        if(ec || !m_ioHandler)
        {
          return;
        }
        queryNextContact();
      }));
}

void Kernel::queryNextContact()
{
  if(!m_ioHandler)
  {
    return;
  }

  const unsigned int total = m_config.s88amount * 16;
  if(m_s88NextContact > total)
  {
    startS88Cycle();
    return;
  }

  m_s88LastQueried = m_s88NextContact;

  // Mark "waiting for reply" and start the response watchdog only once the
  // query has actually been written to the wire. When the command queue is
  // enabled the query may sit behind other frames, so arming the 1 s watchdog
  // at real send time (not enqueue time) keeps it measuring the device, not
  // the queue delay.
  auto onSent = [this]()
  {
    m_s88WaitingReply = true;

    // Safety net: if the device doesn't respond within the timeout,
    // skip this contact and continue — prevents the cycle from hanging.
    m_s88ResponseTimer.expires_after(kS88ResponseTimeout);
    m_s88ResponseTimer.async_wait(
      m_strand.wrap(
        [this](const boost::system::error_code& ec)
        {
          if(ec) // cancelled normally (response arrived)
          {
            return;
          }
          onS88ResponseTimeout();
        }));
  };

  std::string cmd = "C " + std::to_string(m_s88NextContact);
  if(m_config.commandQueue)
  {
    // S88 queries share the single FIFO with commands (one physical line).
    enqueueCmd(std::move(cmd), onSent);
  }
  else
  {
    writeCmdNow(cmd);
    onSent();
  }
}

void Kernel::onS88Response(const std::string& line)
{
  // Cancel the timeout — we got a reply.
  m_s88ResponseTimer.cancel();
  m_s88WaitingReply = false;

  bool state = false;
  try
  {
    state = (std::stoi(line) != 0);
  }
  catch(...)
  {
  }

  const uint32_t contact = m_s88NextContact++;

  if(s88Callback)
  {
    EventLoop::call(
      [this, contact, state]()
      {
        if(s88Callback)
        {
          s88Callback(contact, state);
        }
      });
  }

  queryNextContact();
}

void Kernel::onS88ResponseTimeout()
{
  // No response within kS88ResponseTimeout — log and move on.
  const uint32_t contact = m_s88LastQueried;
  EventLoop::call(
    [this, contact]()
    {
      Log::log(logId, LogMessage::W9999_X,
               "S88 no response for contact " + std::to_string(contact) +
               ", skipping");
    });

  m_s88WaitingReply = false;
  m_s88NextContact++;
  queryNextContact();
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

} // namespace Marklin6023
