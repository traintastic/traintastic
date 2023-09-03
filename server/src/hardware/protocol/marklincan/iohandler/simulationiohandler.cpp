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
#include "../message/statusdataconfig.hpp"
#include "../../../../utils/random.hpp"
#include "../../../../utils/zlib.hpp"

namespace MarklinCAN {

SimulationIOHandler::SimulationIOHandler(Kernel& kernel)
  : IOHandler(kernel)
  , m_pingTimer{kernel.ioContext()}
  , m_bootloaderCANTimer{kernel.ioContext()}
  , m_delayedMessageTimer{kernel.ioContext()}
{
}

void SimulationIOHandler::start()
{
  using namespace std::chrono_literals;
  auto expireAfter = std::chrono::milliseconds(Random::value<int>(0, bootstrapCANInterval.count()));
  startBootloaderCANTimer(expireAfter);
  startPingTimer(expireAfter + 1s);
}

void SimulationIOHandler::startBootloaderCANTimer(std::chrono::milliseconds expireAfter)
{
  m_bootloaderCANTimer.expires_after(expireAfter);
  m_bootloaderCANTimer.async_wait(
    [this](const boost::system::error_code& ec)
    {
      if(!ec)
      {
        reply(BootloaderCAN(guiUID));
      }

      if(ec != boost::asio::error::operation_aborted)
      {
        startBootloaderCANTimer();
      }
    });
}

void SimulationIOHandler::startPingTimer(std::chrono::milliseconds expireAfter)
{
  m_pingTimer.expires_after(expireAfter);
  m_pingTimer.async_wait(
    [this](const boost::system::error_code& ec)
    {
      if(!ec)
      {
        reply(Ping(guiUID));
        replyPing();
      }

      if(ec != boost::asio::error::operation_aborted)
      {
        startPingTimer();
      }
    });
}

void SimulationIOHandler::stop()
{
  while(!m_delayedMessages.empty())
    m_delayedMessages.pop();

  m_bootloaderCANTimer.cancel();
  m_pingTimer.cancel();
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
      // not (yet) implemented
      break;

    case Command::Ping:
      if(message.dlc == 0 && !message.isResponse())
      {
        replyPing();
        reply(PingReply(guiUID, 4, 3, DeviceId::CS2GUI));
      }
      else if(message.dlc == 8 && message.isResponse())
      {
        const auto& pingReply = static_cast<const PingReply&>(message);
        if(m_knownPingUIDs.count(pingReply.uid()) == 0)
        {
          m_knownPingUIDs.emplace(pingReply.uid());
          reply(StatusDataConfig(guiUID, pingReply.uid(), 0));
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
      if(message.dlc == 5 && !message.isResponse())
      {
        const uint32_t uid = static_cast<const UidMessage&>(message).uid();
        const uint8_t index = message.data[4];
        switch(index)
        {
          case 0x00:
          {
            StatusData::DeviceDescription desc;
            switch(uid)
            {
              case gfpUID:
                desc.numberOfReadings = 4;
                desc.numberOfConfigurationChannels = 2;
                desc.serialNumber = 12345;
                desc.articleNumber[0] = '6';
                desc.articleNumber[1] = '0';
                desc.articleNumber[2] = '2';
                desc.articleNumber[3] = '1';
                desc.articleNumber[4] = '4';
                desc.deviceName = "Central Station 2";
                break;

              case linkS88UID:
                desc.numberOfReadings = 0;
                desc.numberOfConfigurationChannels = 12;
                desc.serialNumber = 1234;
                desc.articleNumber[0] = '6';
                desc.articleNumber[1] = '0';
                desc.articleNumber[2] = '8';
                desc.articleNumber[3] = '8';
                desc.articleNumber[4] = '3';
                desc.deviceName = "Link S88";
                break;

              case guiUID: // CS2 GUI doesn't respond to this request
              default:
                // no response
                return true;
            }
            for(const auto& msg : statusDataConfigReply(uid, uid, index, desc))
              reply(msg);
            break;
          }
        }
      }
      break;

    case Command::ConfigData:
      if(message.dlc == 8 && !message.isResponse())
      {
        const auto& configData = static_cast<const ConfigData&>(message);

        if(configData.name() == ConfigDataName::loks)
        {
          static constexpr std::string_view emptyLoks = "[lokomotive]\nversion\n .minor=4\nsession\n .id=13749\n";

          // compress:
          std::vector<std::byte> data;
          data.resize(emptyLoks.size());
          if(!ZLib::compressString(emptyLoks, data))
            data.resize(data.size() * 2); // retry with larger buffer
          if(!ZLib::compressString(emptyLoks, data))
            break;

          // prepend uncompressed size (big endian):
          uint32_t uncompressedSize = host_to_be<uint32_t>(emptyLoks.size());
          for(int i = sizeof(uncompressedSize) - 1; i >= 0; i--)
            data.insert(data.begin(), reinterpret_cast<const std::byte*>(&uncompressedSize)[i]);

          reply(ConfigData(guiUID, configData.name(), true));
          reply(ConfigDataStream(guiUID, data.size(), crc16(data)));

          const std::byte* end = data.data() + data.size();
          for(std::byte* p = data.data(); p < end; p += 8)
          {
            reply(ConfigDataStream(guiUID, p, std::min<size_t>(end - p, 8)));
          }
        }
      }
      break;

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

void SimulationIOHandler::replyPing()
{
  reply(PingReply(gfpUID, 3, 85, DeviceId::GleisFormatProzessorOrBooster));
  reply(PingReply(linkS88UID, 1, 1, static_cast<DeviceId>(64)));
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
