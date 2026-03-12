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
#include <string>

using namespace std::chrono_literals;

namespace Marklin6023 {

static constexpr auto kS88ResponseTimeout = std::chrono::milliseconds(1000);
static std::string interpretTx(const std::string& cmd)
{
  if(cmd.empty())
    return cmd;

  // "G" – Global Go
  if(cmd == "G")
    return "Global go";

  // "S" – Global Stop
  if(cmd == "S")
    return "Global stop";

  // "L <addr> S <spd> F <f0>" – loco speed + F0
  // "L <addr> D"              – loco direction toggle
  if(cmd.size() >= 2 && cmd[0] == 'L' && cmd[1] == ' ')
  {
    // find addr (after "L ")
    const std::size_t addrStart = 2;
    const std::size_t spaceAfterAddr = cmd.find(' ', addrStart);
    if(spaceAfterAddr == std::string::npos)
      return cmd;
    const std::string addr = cmd.substr(addrStart, spaceAfterAddr - addrStart);
    const std::string rest = cmd.substr(spaceAfterAddr + 1);

    if(!rest.empty() && rest[0] == 'D')
      return "Loco " + addr + ": direction toggle";

    // "S <spd> F <f0>"
    // rest = "S 5 F 0"
    unsigned int spd = 0, f0 = 0;
    if(std::sscanf(rest.c_str(), "S %u F %u", &spd, &f0) == 2)
      return "Loco " + addr + ": speed " + std::to_string(spd) +
                   ", F0=" + (f0 ? "on" : "off");

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

Kernel::Kernel(std::string logId_, const Config& config)
  : KernelBase{std::move(logId_)}
  , m_config{config}
  , m_strand{m_ioContext}
  , m_s88Timer{m_ioContext}
  , m_s88ResponseTimer{m_ioContext}
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
  startS88Cycle();
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
      if(m_ioHandler)
    m_ioHandler->stop();
      m_ioContext.stop();
  });

  if(m_ioThread.joinable())
  m_ioThread.join();
}

void Kernel::sendGlobalGo()
{
  m_strand.post([this](){ sendCmdWithRedundancy("G"); });
}

void Kernel::sendGlobalStop()
{
  m_strand.post([this](){ sendCmdWithRedundancy("S"); });
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
      const std::string cmd = "L " + std::to_string(address) + " D";
      sendCmd(cmd);

      auto& t = m_redundancyTimers.emplace_back(m_ioContext);
      t.expires_after(50ms);
      t.async_wait(
        m_strand.wrap(
          [this, cmd](const boost::system::error_code& ec)
          {
            if(!ec && m_ioHandler)
              sendCmd(cmd);
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
    return false;

  char dir = 'G';
  std::visit(
    [&](auto&& v)
    {
      using T = std::decay_t<decltype(v)>;
      if constexpr(std::is_same_v<T, OutputPairValue>)
        dir = (v == OutputPairValue::First) ? 'R' : 'G';
      else if constexpr(std::is_same_v<T, TriState>)
        dir = (v == TriState::True) ? 'R' : 'G';
    }, value);

  m_strand.post(
    [this, address, dir]()
    {
      sendCmdWithRedundancy("M " + std::to_string(address) + " " + dir);
    });

  return true;
}

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
    onS88Response(line);
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

void Kernel::sendCmd(std::string cmd)
{
  if(!m_ioHandler)
    return;

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

  m_ioHandler->sendString(std::move(cmd) + CR);
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
            sendCmd(cmd);
        }));
  }
}

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
          return;
        queryNextContact();
      }));
}

void Kernel::queryNextContact()
{
  if(!m_ioHandler)
    return;

  const unsigned int total = m_config.s88amount * 16;
  if(m_s88NextContact > total)
  {
    startS88Cycle();
    return;
  }

  m_s88LastQueried  = m_s88NextContact;
  m_s88WaitingReply = true;
  sendCmd("C " + std::to_string(m_s88NextContact));

  // Safety net: if the device doesn't respond within the timeout,
  // skip this contact and continue — prevents the cycle from hanging.
  m_s88ResponseTimer.expires_after(kS88ResponseTimeout);
  m_s88ResponseTimer.async_wait(
    m_strand.wrap(
      [this](const boost::system::error_code& ec)
      {
        if(ec) // cancelled normally (response arrived)
          return;
        onS88ResponseTimeout();
      }));
}

void Kernel::onS88Response(const std::string& line)
{
  // Cancel the timeout — we got a reply.
  m_s88ResponseTimer.cancel();
  m_s88WaitingReply = false;

  bool state = false;
  try { state = (std::stoi(line) != 0); }
  catch(...) {}

  const uint32_t contact = m_s88NextContact++;

  if(s88Callback)
  {
    EventLoop::call(
      [this, contact, state]()
      {
        if(s88Callback)
          s88Callback(contact, state);
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

} // namespace Marklin6023
