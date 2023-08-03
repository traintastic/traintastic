/**
 * server/src/hardware/protocol/marklincan/iohandler/simulationiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "simulationiohandler.hpp"
#include "../kernel.hpp"

namespace MarklinCAN {

SimulationIOHandler::SimulationIOHandler(Kernel& kernel)
  : IOHandler(kernel)
  , m_delayedMessageTimer{kernel.ioContext()}
{
}

void SimulationIOHandler::stop()
{
  while(!m_delayedMessages.empty())
    m_delayedMessages.pop();

  m_delayedMessageTimer.cancel();
}

bool SimulationIOHandler::send(const Message& message)
{
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
        case SystemSubCommand::LocomotiveEmergencyStop:
        case SystemSubCommand::LocomotiveCycleEnd:
          // not (yet) implemented
          break;

        case SystemSubCommand::AccessorySwitchTime:
          if(!message.isResponse() && message.dlc == 7)
          {
            auto response = static_cast<const AccessorySwitchTime&>(message);
            if(response.switchTime() == 0)
              m_switchTime = defaultSwitchTime;
            else
              m_switchTime = std::chrono::milliseconds(response.switchTime() * 10);
            response.setResponse(true);
            reply(response);
          }
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
    case Command::LocomotiveSpeed:
    case Command::LocomotiveDirection:
    case Command::LocomotiveFunction:
    case Command::ReadConfig:
    case Command::WriteConfig:
      // not (yet) implemented
      break;

    case Command::AccessoryControl:
      if(!message.isResponse() && (message.dlc == 6 || message.dlc == 8))
      {
        // confirm switch command:
        AccessoryControl response{static_cast<const AccessoryControl&>(message)};
        response.setResponse(true);
        reply(response);

        // handle switch time:
        if(response.current() != 0 && (response.isDefaultSwitchTime() || response.switchTime() > 0))
        {
          const auto switchTime = response.isDefaultSwitchTime() ? m_switchTime : std::chrono::milliseconds(response.switchTime() * 10);

          response.setResponse(false);
          response.setCurrent(0);
          reply(response, switchTime);

          response.setResponse(true);
          reply(response, switchTime + std::chrono::milliseconds(1));
        }
      }
      break;

    case Command::AccessoryConfig:
    case Command::S88Polling:
    case Command::FeedbackEvent:
    case Command::SX1Event:
    case Command::Ping:
    case Command::Update:
    case Command::ReadConfigData:
    case Command::BootloaderCAN:
    case Command::BootloaderTrack:
    case Command::StatusDataConfig:
    case Command::ConfigData:
    case Command::ConfigDataStream:
      // not (yet) implemented
      break;
  }
  return true;
}

void SimulationIOHandler::reply(const Message& message)
{
  // post the reply, so it has some delay
  m_kernel.ioContext().post(
    [this, message]()
    {
      m_kernel.receive(message);
    });
}

void SimulationIOHandler::reply(const Message& message, std::chrono::milliseconds delay)
{
  const auto time = std::chrono::steady_clock::now() + delay;
  const bool restartTimer = m_delayedMessages.empty() || m_delayedMessages.top().time > time;
  m_delayedMessages.push({time, message});
  if(restartTimer)
    restartDelayedMessageTimer();
}

void SimulationIOHandler::restartDelayedMessageTimer()
{
  assert(!m_delayedMessages.empty());
  m_delayedMessageTimer.cancel();
  m_delayedMessageTimer.expires_after(m_delayedMessages.top().time - std::chrono::steady_clock::now());
  m_delayedMessageTimer.async_wait(
    [this](const boost::system::error_code& ec)
    {
      if(!ec)
      {
        while(!m_delayedMessages.empty() && m_delayedMessages.top().time <= std::chrono::steady_clock::now())
        {
          reply(m_delayedMessages.top().message);
          m_delayedMessages.pop();
        }
      }

      if(ec != boost::asio::error::operation_aborted && !m_delayedMessages.empty())
        restartDelayedMessageTimer();
    });
}

}
