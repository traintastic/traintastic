/**
 * server/src/hardware/protocol/z21/serverkernel.cpp
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

#include "serverkernel.hpp"
#include "messages.hpp"
#include "../xpressnet/messages.hpp"
#include "../../decoder/list/decoderlist.hpp"
#include "../../protocol/dcc/dcc.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"

namespace Z21 {

ServerKernel::ServerKernel(std::string logId_, const ServerConfig& config, std::shared_ptr<DecoderList> decoderList)
  : Kernel(std::move(logId_))
  , m_inactiveClientPurgeTimer{m_ioContext}
  , m_config{config}
  , m_decoderList{std::move(decoderList)}
{
}

void ServerKernel::setConfig(const ServerConfig& config)
{
  m_ioContext.post(
    [this, newConfig=config]()
    {
      m_config = newConfig;
    });
}

void ServerKernel::setState(bool trackPowerOn, bool emergencyStop)
{
  m_ioContext.post(
    [this, trackPowerOn, emergencyStop]()
    {
      const auto trackPowerOnTri = toTriState(trackPowerOn);
      const auto emergencyStopTri = toTriState(emergencyStop);

      const bool trackPowerOnChanged = m_trackPowerOn != trackPowerOnTri;
      const bool emergencyStopChanged = m_emergencyStop != emergencyStopTri;

      m_trackPowerOn = trackPowerOnTri;
      m_emergencyStop = emergencyStopTri;

      if(emergencyStopChanged && m_emergencyStop == TriState::True)
        sendTo(LanXBCStopped(), BroadcastFlags::PowerLocoTurnoutChanges);

      if(trackPowerOnChanged && m_trackPowerOn == TriState::False)
        sendTo(LanXBCTrackPowerOff(), BroadcastFlags::PowerLocoTurnoutChanges);

      if((trackPowerOnChanged || emergencyStopChanged) && m_trackPowerOn == TriState::True && m_emergencyStop == TriState::False)
        sendTo(LanXBCTrackPowerOn(), BroadcastFlags::PowerLocoTurnoutChanges);

      if(trackPowerOnChanged || emergencyStopChanged)
        sendTo(getLanSystemStateDataChanged(), BroadcastFlags::PowerLocoTurnoutChanges);
    });
}

void ServerKernel::receiveFrom(const Message& message, IOHandler::ClientId clientId)
{
  if(m_config.debugLogRXTX)
    EventLoop::call(
      [this, clientId, msg=toString(message)]()
      {
        Log::log(logId, LogMessage::D2005_X_RX_X, clientId, msg);
      });

  m_clients[clientId].lastSeen = std::chrono::steady_clock::now();

  switch(message.header())
  {
    case LAN_X:
    {
      const auto& lanX = static_cast<const LanX&>(message);

      if(!XpressNet::isChecksumValid(*reinterpret_cast<const XpressNet::Message*>(&lanX.xheader)))
        break;

      switch(lanX.xheader)
      {
        case 0x21:
          if(message == LanXGetVersion())
          {
            sendTo(LanXGetVersionReply(ServerConfig::xBusVersion, ServerConfig::commandStationId), clientId);
          }
          else if(message == LanXGetStatus())
          {
            LanXStatusChanged response;
            if(m_emergencyStop != TriState::False)
              response.db1 |= Z21_CENTRALSTATE_EMERGENCYSTOP;
            if(m_trackPowerOn != TriState::True)
              response.db1 |= Z21_CENTRALSTATE_TRACKVOLTAGEOFF;
            response.updateChecksum();
            sendTo(response, clientId);
          }
          else if(message == LanXSetTrackPowerOn())
          {
            if(m_config.allowTrackPowerOnReleaseEmergencyStop && (m_trackPowerOn != TriState::True || m_emergencyStop != TriState::False) && m_onTrackPowerOn)
            {
              EventLoop::call(
                [this]()
                {
                  m_onTrackPowerOn();
                });
            }
          }
          else if(message == LanXSetTrackPowerOff())
          {
            if(m_config.allowTrackPowerOff && m_trackPowerOn != TriState::False && m_onTrackPowerOff)
            {
              EventLoop::call(
                [this]()
                {
                  m_onTrackPowerOff();
                });
            }
          }
          break;

        case LAN_X_SET_STOP:
          if(message == LanXSetStop())
          {
            if(m_config.allowEmergencyStop && m_emergencyStop != TriState::True && m_onEmergencyStop)
            {
              EventLoop::call(
                [this]()
                {
                  m_onEmergencyStop();
                });
            }
          }
          break;

        case LAN_X_GET_LOCO_INFO:
          if(const auto& getLocoInfo = static_cast<const LanXGetLocoInfo&>(message);
              getLocoInfo.db0 == 0xF0)
          {
            subscribe(clientId, getLocoInfo.address(), getLocoInfo.isLongAddress());

            EventLoop::call(
              [this, getLocoInfo, clientId]()
              {
                if(auto decoder = getDecoder(getLocoInfo.address(), getLocoInfo.isLongAddress()))
                  postSendTo(LanXLocoInfo(*decoder), clientId);
              });
          }
          break;

        case LAN_X_SET_LOCO:
          if(const auto& setLocoDrive = static_cast<const LanXSetLocoDrive&>(message);
              setLocoDrive.db0 >= 0x10 && setLocoDrive.db0 <= 0x13)
          {
            subscribe(clientId, setLocoDrive.address(), setLocoDrive.isLongAddress());

            EventLoop::call(
              [this, setLocoDrive]()
              {
                if(auto decoder = getDecoder(setLocoDrive.address(), setLocoDrive.isLongAddress()))
                {
                  decoder->direction = setLocoDrive.direction();
                  decoder->emergencyStop = setLocoDrive.isEmergencyStop();
                  decoder->throttle = Decoder::speedStepToThrottle(setLocoDrive.speedStep(), setLocoDrive.speedSteps());
                }
                //else
                //  Log::log(*this, LogMessage::I2001_UNKNOWN_LOCO_ADDRESS_X, setLocoDrive.address());
              });
          }
          else if(const auto& setLocoFunction = static_cast<const LanXSetLocoFunction&>(message);
                  setLocoFunction.db0 == 0xF8 &&
                  setLocoFunction.switchType() != LanXSetLocoFunction::SwitchType::Invalid)
          {
            subscribe(clientId, setLocoFunction.address(), setLocoFunction.isLongAddress());

            EventLoop::call(
              [this, setLocoFunction]()
              {
                if(auto decoder = getDecoder(setLocoFunction.address(), setLocoFunction.isLongAddress()))
                {
                  if(auto function = decoder->getFunction(setLocoFunction.functionIndex()))
                  {
                    switch(setLocoFunction.switchType())
                    {
                      case LanXSetLocoFunction::SwitchType::Off:
                        function->value = false;
                        break;

                      case LanXSetLocoFunction::SwitchType::On:
                        function->value = true;
                        break;

                      case LanXSetLocoFunction::SwitchType::Toggle:
                        function->value = !function->value;
                        break;

                      case LanXSetLocoFunction::SwitchType::Invalid:
                        assert(false);
                        break;
                    }
                  }
                }
              });
          }
          break;

        case LAN_X_GET_FIRMWARE_VERSION:
          if(message == LanXGetFirmwareVersion())
            sendTo(LanXGetFirmwareVersionReply(ServerConfig::firmwareVersionMajor, ServerConfig::firmwareVersionMinor), clientId);
          break;
      }
      break;
    }
    case LAN_GET_LOCO_MODE:
      if(message.dataLen() == sizeof(LanGetLocoMode))
        sendTo(LanGetLocoModeReply(static_cast<const LanGetLocoMode&>(message).address(), LocoMode::DCC), clientId);
      break;

    case LAN_SET_LOCO_MODE:
      // ignore, we always report DCC
      break;

    case LAN_GET_SERIAL_NUMBER:
      if(message.dataLen() == sizeof(LanGetSerialNumber))
        sendTo(LanGetSerialNumberReply(ServerConfig::serialNumber), clientId);
      break;

    case LAN_GET_HWINFO:
      if(message.dataLen() == sizeof(LanGetHardwareInfo))
        sendTo(LanGetHardwareInfoReply(ServerConfig::hardwareType, ServerConfig::firmwareVersionMajor, ServerConfig::firmwareVersionMinor), clientId);
      break;

    case LAN_GET_BROADCASTFLAGS:
      if(message == LanGetBroadcastFlags())
        sendTo(LanSetBroadcastFlags(m_clients[clientId].broadcastFlags), clientId);
      break;

    case LAN_SET_BROADCASTFLAGS:
      if(message.dataLen() == sizeof(LanSetBroadcastFlags))
        m_clients[clientId].broadcastFlags = static_cast<const LanSetBroadcastFlags&>(message).broadcastFlags();
      break;

    case LAN_SYSTEMSTATE_GETDATA:
      if(message == LanSystemStateGetData())
        sendTo(getLanSystemStateDataChanged(), clientId);
      break;

    case LAN_LOGOFF:
      if(message == LanLogoff())
        m_clients.erase(clientId);
      break;

    case LAN_GET_CODE:
    case LAN_GET_TURNOUTMODE:
    case LAN_SET_TURNOUTMODE:
    case LAN_RMBUS_DATACHANGED:
    case LAN_RMBUS_GETDATA:
    case LAN_RMBUS_PROGRAMMODULE:
    case LAN_SYSTEMSTATE_DATACHANGED:
    case LAN_RAILCOM_DATACHANGED:
    case LAN_RAILCOM_GETDATA:
    case LAN_LOCONET_Z21_RX:
    case LAN_LOCONET_Z21_TX:
    case LAN_LOCONET_FROM_LAN:
    case LAN_LOCONET_DISPATCH_ADDR:
    case LAN_LOCONET_DETECTOR:
    case LAN_CAN_DETECTOR:
      break; // not (yet) supported
  }
}

void ServerKernel::onStart()
{
  startInactiveClientPurgeTimer();
}

void ServerKernel::onStop()
{
  m_inactiveClientPurgeTimer.cancel();

  for(auto& it : m_decoderSubscriptions)
    it.second.connection.disconnect();
}

void ServerKernel::sendTo(const Message& message, IOHandler::ClientId clientId)
{
  if(m_ioHandler->sendTo(message, clientId))
  {
    if(m_config.debugLogRXTX)
      EventLoop::call(
        [this, clientId, msg=toString(message)]()
        {
          Log::log(logId, LogMessage::D2004_X_TX_X, clientId, msg);
        });
  }
  else
  {} // log message and go to error state
}

void ServerKernel::sendTo(const Message& message, BroadcastFlags broadcastFlags)
{
  for(const auto& client : m_clients)
    if((client.second.broadcastFlags & broadcastFlags) != BroadcastFlags::None)
      sendTo(message, client.first);
}

LanSystemStateDataChanged ServerKernel::getLanSystemStateDataChanged() const
{
  LanSystemStateDataChanged message;

  if(m_emergencyStop != TriState::False)
    message.centralState |= Z21_CENTRALSTATE_EMERGENCYSTOP;
  if(m_trackPowerOn != TriState::True)
    message.centralState |= Z21_CENTRALSTATE_TRACKVOLTAGEOFF;

  return message;
}

std::shared_ptr<Decoder> ServerKernel::getDecoder(uint16_t address, bool longAddress) const
{
  auto decoder = m_decoderList->getDecoder(longAddress ? DecoderProtocol::DCCLong : DecoderProtocol::DCCShort, address);
  if(!decoder)
    decoder = m_decoderList->getDecoder(address);
  return decoder;
}

void ServerKernel::removeClient(IOHandler::ClientId clientId)
{
  auto& subscriptions = m_clients[clientId].subscriptions;
  while(!subscriptions.empty())
    unsubscribe(clientId, *subscriptions.begin());
  m_clients.erase(clientId);
  m_ioHandler->purgeClient(clientId);
}

void ServerKernel::subscribe(IOHandler::ClientId clientId, uint16_t address, bool longAddress)
{
  auto& subscriptions = m_clients[clientId].subscriptions;
  const std::pair<uint16_t, bool> key{address, longAddress};
  if(std::find(subscriptions.begin(), subscriptions.end(), key) != subscriptions.end())
    return;
  subscriptions.emplace_back(address, longAddress);
  if(subscriptions.size() > ServerConfig::subscriptionMax)
    unsubscribe(clientId, *subscriptions.begin());

  EventLoop::call(
    [this, key]()
    {
      if(auto it = m_decoderSubscriptions.find(key); it == m_decoderSubscriptions.end())
      {
        if(auto decoder = getDecoder(key.first, key.second))
          m_decoderSubscriptions.emplace(key, DecoderSubscription{decoder->decoderChanged.connect(std::bind(&ServerKernel::decoderChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), 1});
      }
      else
      {
        it->second.count++;
      }
    });
}

void ServerKernel::unsubscribe(IOHandler::ClientId clientId, std::pair<uint16_t, bool> key)
{
  {
    auto& subscriptions = m_clients[clientId].subscriptions;
    auto it = std::find(subscriptions.begin(), subscriptions.end(), key);
    if(it != subscriptions.end())
      subscriptions.erase(it);
  }

  EventLoop::call(
    [this, key]()
    {
      if(auto it = m_decoderSubscriptions.find(key); it != m_decoderSubscriptions.end())
      {
        assert(it->second.count > 0);
        if(--it->second.count == 0)
          m_decoderSubscriptions.erase(it);
      }
    });
}

void ServerKernel::decoderChanged(const Decoder& decoder, DecoderChangeFlags /*changes*/, uint32_t /*functionNumber*/)
{
  const std::pair<uint16_t, bool> key(decoder.address, decoder.protocol == DecoderProtocol::DCCLong);
  const LanXLocoInfo message(decoder);

  EventLoop::call(
    [this, key, message]()
    {
      for(auto it : m_clients)
        if((it.second.broadcastFlags & BroadcastFlags::PowerLocoTurnoutChanges) == BroadcastFlags::PowerLocoTurnoutChanges)
        {
          auto& subscriptions = it.second.subscriptions;
          if(std::find(subscriptions.begin(), subscriptions.end(), key) != subscriptions.end())
            sendTo(message, it.first);
        }
    });
}

void ServerKernel::startInactiveClientPurgeTimer()
{
  assert(ServerConfig::inactiveClientPurgeTime > 0);
  m_inactiveClientPurgeTimer.expires_after(boost::asio::chrono::seconds(std::max(1, ServerConfig::inactiveClientPurgeTime / 4)));
  m_inactiveClientPurgeTimer.async_wait(std::bind(&ServerKernel::inactiveClientPurgeTimerExpired, this, std::placeholders::_1));
}

void ServerKernel::inactiveClientPurgeTimerExpired(const boost::system::error_code& ec)
{
  if(ec)
    return;

  std::vector<IOHandler::ClientId> clientsToRemove;
  const auto purgeTime = std::chrono::steady_clock::now() - std::chrono::seconds(ServerConfig::inactiveClientPurgeTime);
  for(const auto& it : m_clients)
    if(it.second.lastSeen < purgeTime)
      clientsToRemove.emplace_back(it.first);

  for(const auto& clientId : clientsToRemove)
    removeClient(clientId);

  startInactiveClientPurgeTimer();
}

}
