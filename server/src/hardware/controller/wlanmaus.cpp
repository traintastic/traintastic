/**
 * server/src/hardware/controller/wlanmaus.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "wlanmaus.hpp"
#include "../../core/traintastic.hpp"
#include "../../core/eventloop.hpp"
#include "../commandstation/commandstation.hpp"
#include "../decoder/decoder.hpp"
#include "../protocol/z21.hpp"
#include "../../utils/to_hex.hpp"
#include "../../core/attributes.hpp"

WLANmaus::WLANmaus(const std::weak_ptr<World> world, std::string_view _id) :
  Controller(world, _id),
  m_socket{Traintastic::instance->ioContext()},
  m_blockLocoInfo{nullptr},
  port{this, "port", 21105, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  Attributes::addEnabled(port, !active);
  m_interfaceItems.add(port);
}

bool WLANmaus::setActive(bool& value)
{
  if(!m_socket.is_open() && value)
  {
    boost::system::error_code ec;

    if(m_socket.open(boost::asio::ip::udp::v4(), ec))
    {
      logError("socket.open: " + ec.message());
      return false;
    }
    else if(m_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), port), ec))
    {
      m_socket.close();
      logError("socket.bind: " + ec.message());
      return false;
    }

    receive(); // start receiving messages

    // TODO: send message were alive ??

    port.setAttributeEnabled(false);
  }
  else if(m_socket.is_open() && !value)
  {
    port.setAttributeEnabled(true);

    m_socket.close();
  }
  return true;
}

void WLANmaus::emergencyStopChanged(bool value)
{
  if(value)
  {
    const z21_lan_x_bc_stopped message;
    for(auto it : m_clients)
      if(it.second.broadcastFlags & Z21::PowerLocoTurnout)
        sendTo(message, it.first);
  }
  else if(commandStation && !commandStation->trackVoltageOff) // send z21_lan_x_bc_track_power_on if power is on
  {
    const z21_lan_x_bc_track_power_on message;
    for(auto it : m_clients)
      if(it.second.broadcastFlags & Z21::PowerLocoTurnout)
        sendTo(message, it.first);
  }
}

void WLANmaus::trackPowerChanged(bool value)
{
  if(value)
  {
    const z21_lan_x_bc_track_power_on message;
    for(auto it : m_clients)
      if(it.second.broadcastFlags & Z21::PowerLocoTurnout)
        sendTo(message, it.first);
  }
  else
  {
    const z21_lan_x_bc_track_power_off message;
    for(auto it : m_clients)
      if(it.second.broadcastFlags & Z21::PowerLocoTurnout)
        sendTo(message, it.first);
  }
}

void WLANmaus::decoderChanged(const Decoder& decoder, DecoderChangeFlags, uint32_t)
{
  if(&decoder == m_blockLocoInfo)
    return;

  //logDebug("loco info: speedStep=" + std::to_string(decoder.speedStep.value()));

  EventLoop::call(
    [this, dec=decoder.shared_ptr_c<const Decoder>()]()
    {
      broadcastLocoInfo(*dec);
    });
}

void WLANmaus::receive()
{
  m_socket.async_receive_from(boost::asio::buffer(m_receiveBuffer), m_receiveEndpoint,
    [this](const boost::system::error_code& ec, std::size_t bytesReceived)
    {
      if(!ec)
      {
        if((bytesReceived >= sizeof(Z21::Message)))
        {
          bool unknownMessage = false;
          const Z21::Message* message = reinterpret_cast<const Z21::Message*>(m_receiveBuffer.data());
          /*[[deprecated]]*/ const z21_lan_header* cmd = reinterpret_cast<const z21_lan_header*>(m_receiveBuffer.data());
          switch(message->header())
          {
            case Z21::LAN_X:
            {
              // TODO check XOR
              const uint8_t xheader = static_cast<const z21_lan_x*>(cmd)->xheader;
              switch(xheader)
              {
                case 0x21:
                  if(*cmd == z21_lan_x_get_status())
                  {
                    EventLoop::call(
                      [this, endpoint=m_receiveEndpoint]()
                      {
                        z21_lan_x_status_changed response;

                        if(!commandStation || commandStation->emergencyStop)
                          response.db1 |= Z21_CENTRALSTATE_EMERGENCYSTOP;
                        if(!commandStation || commandStation->trackVoltageOff)
                          response.db1 |= Z21_CENTRALSTATE_TRACKVOLTAGEOFF;

                        response.calcChecksum();

                        sendTo(response, endpoint);
                      });
                  }
                  else if(*cmd == z21_lan_x_set_track_power_on())
                  {
                    EventLoop::call(
                      [this, endpoint=m_receiveEndpoint]()
                      {
                        if(commandStation)
                        {
                          commandStation->emergencyStop = false;
                          commandStation->trackVoltageOff = false;
                          if(!commandStation->trackVoltageOff)
                            sendTo(z21_lan_x_bc_track_power_on(), endpoint);
                        }
                      });
                  }
                  else if(*cmd == z21_lan_x_set_track_power_off())
                  {
                    EventLoop::call(
                      [this, endpoint=m_receiveEndpoint]()
                      {
                        if(commandStation)
                        {
                          commandStation->trackVoltageOff = true;
                          if(commandStation->trackVoltageOff)
                            sendTo(z21_lan_x_bc_track_power_off(), endpoint);
                        }
                      });
                  }
                  else
                    unknownMessage = true;
                  break;

                case 0x80:
                  if(*cmd == z21_lan_x_set_stop())
                  {
                    EventLoop::call(
                      [this, endpoint=m_receiveEndpoint]()
                      {
                        if(commandStation)
                        {
                          commandStation->emergencyStop = true;
                          if(commandStation->emergencyStop)
                            sendTo(z21_lan_x_bc_stopped(), endpoint);
                        }
                      });
                  }
                  else
                    unknownMessage = true;
                  break;

                case 0xE3:
                  if(const z21_lan_x_get_loco_info* r = static_cast<const z21_lan_x_get_loco_info*>(cmd);
                      r->db0 == 0xF0)
                  {
                    EventLoop::call(
                      [this, request=*r, endpoint=m_receiveEndpoint]()
                      {
                        if(commandStation)
                          if(auto decoder = commandStation->getDecoder(DecoderProtocol::DCC, request.address(), request.isLongAddress()))
                          {
//std::stringstream ss;
//ss << endpoint;
//
  //                          logDebug("locoinfo endpoint=" + ss.str());
                            m_clients[endpoint].locoInfo.insert(locoInfoKey(request.address(), request.isLongAddress()));
    //                        logDebug("m_clients[endpoint].broadcastFlags = " + std::to_string(m_clients[endpoint].broadcastFlags));
      //                      logDebug("m_clients[endpoint].locoInfo.size() = " + std::to_string(m_clients[endpoint].locoInfo.size()));
        //                    logDebug("m_clients.size() = " + std::to_string(m_clients.size()));
                            sendTo(z21_lan_x_loco_info(*decoder), endpoint);
                          }
                      });
                  }
                  else
                    unknownMessage = true;
                  break;

                case 0xE4:
                  if(const z21_lan_x_set_loco_drive* r = static_cast<const z21_lan_x_set_loco_drive*>(cmd);
                      r->db0 >= 0x10 && r->db0 <= 0x13)
                  {
                    EventLoop::call(
                      [this, request=*r]()
                      {
                        if(!commandStation)
                          return;

                        if(auto decoder = commandStation->getDecoder(DecoderProtocol::DCC, request.address(), request.isLongAddress()))
                        {
                          //logDebug("loco drive: speedStep=" + std::to_string(decoder->speedStep.value()));
                       //   logDebug("z21_lan_x_set_loco_drive.speedAndDirection=" + std::to_string(request.speedAndDirection));


                          //m_blockLocoInfo = decoder.get();
                          decoder->direction = request.direction();
                          decoder->emergencyStop = request.isEmergencyStop();
                          if(decoder->speedSteps == request.speedSteps())
                            decoder->speedStep = request.speedStep();
                          else
                            decoder->speedStep = std::round(decoder->speedSteps * request.speedStep() / static_cast<float>(request.speedSteps()));
                          //broadcastLocoInfo(*decoder);
                          //m_blockLocoInfo = nullptr;
                        }
                        else
                          logInfo("Unknown loco address: " + std::to_string(request.address()));
                      });
                  }
                  else if(const z21_lan_x_set_loco_function* r = static_cast<const z21_lan_x_set_loco_function*>(cmd);
                          r->db0 == 0xF8 &&
                          r->switchType() != z21_lan_x_set_loco_function::SwitchType::Invalid)
                  {
                    EventLoop::call(
                      [this, request=*r]()
                      {
                        if(commandStation)
                          if(auto decoder = commandStation->getDecoder(DecoderProtocol::DCC, request.address(), request.isLongAddress()))
                            if(auto function = decoder->getFunction(request.functionIndex()))
                              switch(request.switchType())
                              {
                                case z21_lan_x_set_loco_function::SwitchType::Off:
                                  function->value = false;
                                  break;

                                case z21_lan_x_set_loco_function::SwitchType::On:
                                  function->value = true;
                                  break;

                                case z21_lan_x_set_loco_function::SwitchType::Toggle:
                                  function->value = !function->value;
                                  break;

                                case z21_lan_x_set_loco_function::SwitchType::Invalid:
                                  assert(false);
                                  break;
                              }
                      });
                  }
                  break;

                case 0xF1:
                  if(*cmd == z21_lan_x_get_firmware_version())
                  {
                    EventLoop::call(
                      [this, endpoint=m_receiveEndpoint]()
                      {
                        sendTo(z21_lan_x_get_firmware_version_reply(1, 30), endpoint);
                      });
                  }
                  else
                    unknownMessage = true;
                  break;

                default:
                  unknownMessage = true;
                  break;
              }
              break;
            }
            case Z21::LAN_GET_LOCO_MODE:
              if(message->dataLen() == sizeof(Z21::LanGetLocoMode))
              {
                // TODO: reply without invoking event loop
                EventLoop::call(
                  [this, address=static_cast<const Z21::LanGetLocoMode*>(message)->address(), endpoint=m_receiveEndpoint]()
                  {
                    sendTo(Z21::LanGetLocoModeReply(address, Z21::LocoMode::DCC), endpoint);
                  });
              }
              else
                unknownMessage = true;
              break;

            case Z21::LAN_SET_LOCO_MODE:
              if(message->dataLen() == sizeof(Z21::LanSetLocoMode))
              {
                // ignore, we always report DCC
              }
              else
                unknownMessage = true;
              break;

            case Z21::LAN_GET_SERIAL_NUMBER:
              if(message->dataLen() == sizeof(Z21::LanGetSerialNumber))
              {
                EventLoop::call(
                  [this, endpoint=m_receiveEndpoint]()
                  {
                    sendTo(Z21::LanGetSerialNumberReply(123456789), endpoint);
                  });
              }
              else
                unknownMessage = true;
              break;

            case Z21::LAN_GET_HWINFO:
              if(cmd->dataLen == sizeof(z21_lan_get_hwinfo))
              {
                EventLoop::call(
                  [this, endpoint=m_receiveEndpoint]()
                  {
                    sendTo(z21_lan_get_hwinfo_reply(Z21_HWT_Z21_START, 1, 30), endpoint);
                  });
              }
              else
                unknownMessage = true;
              break;

            case Z21::LAN_SET_BROADCASTFLAGS:
              if(message->dataLen() == sizeof(z21_lan_set_broadcastflags))
              {
                EventLoop::call(
                  [this, request=*static_cast<const z21_lan_set_broadcastflags*>(cmd), endpoint=m_receiveEndpoint]()
                  {
                    m_clients[endpoint].broadcastFlags = request.broadcastFlags;
                  });
              }
              else
                unknownMessage = true;
              break;

            case Z21::LAN_SYSTEMSTATE_GETDATA:
              if(message->dataLen() == sizeof(z21_lan_systemstate_getdata))
              {
                EventLoop::call(
                  [this, endpoint=m_receiveEndpoint]()
                  {
                    z21_lan_systemstate_datachanged response;

                    if(!commandStation || commandStation->emergencyStop)
                      response.centralState |= Z21_CENTRALSTATE_EMERGENCYSTOP;
                    if(!commandStation || commandStation->trackVoltageOff)
                      response.centralState |= Z21_CENTRALSTATE_TRACKVOLTAGEOFF;

                    sendTo(response, endpoint);
                  });
              }
              else
                unknownMessage = true;
              break;

            case Z21::LAN_LOGOFF:
              if(message->dataLen() == sizeof(Z21::LanLogoff))
              {
                EventLoop::call(
                  [this, endpoint=m_receiveEndpoint]()
                  {
                    m_clients.erase(endpoint);
                  });
              }
              else
                unknownMessage = true;
              break;

            default:
              unknownMessage = true;
              break;
          }

          if(unknownMessage /*&& debugEnabled*/)
          {
            std::string message = "unknown message: dataLen=0x" + to_hex(cmd->dataLen) + ", header=0x" + to_hex(cmd->header);
            if(cmd->dataLen > 4)
            {
              message += ", data=";
              for(int i = 4; i < cmd->dataLen; i++)
                message += to_hex(reinterpret_cast<const uint8_t*>(cmd)[i]);
            }
            EventLoop::call([this, message](){ logDebug(message); });
          }
        }
        receive();
      }
      else
        EventLoop::call(
          [this, ec]()
          {
            logError("socket.async_receive_from: " + ec.message());
          });
    });
}

void WLANmaus::sendTo(const z21_lan_header& msg, const boost::asio::ip::udp::endpoint& endpoint)
{
  // TODO: add to queue, send async

  boost::system::error_code ec;
  m_socket.send_to(boost::asio::buffer(&msg, msg.dataLen), endpoint, 0, ec);
  if(ec)
     EventLoop::call([this, ec](){ logError("socket.send_to: " + ec.message()); });
/*
  m_socket.async_send_to(boost::asio::buffer(&msg, msg.dataLen), endpoint,
    [this](const boost::system::error_code& ec, std::size_t)
    {
      if(ec)
         EventLoop::call([this, ec](){ logError("socket.async_send_to: " + ec.message()); });
    });
    */
}

void WLANmaus::sendTo(const Z21::Message& message, const boost::asio::ip::udp::endpoint& endpoint)
{
  // TODO: add to queue, send async

  boost::system::error_code ec;
  m_socket.send_to(boost::asio::buffer(&message, message.dataLen()), endpoint, 0, ec);
  if(ec)
     EventLoop::call([this, ec](){ logError("socket.send_to: " + ec.message()); });
/*
  m_socket.async_send_to(boost::asio::buffer(&msg, msg.dataLen), endpoint,
    [this](const boost::system::error_code& ec, std::size_t)
    {
      if(ec)
         EventLoop::call([this, ec](){ logError("socket.async_send_to: " + ec.message()); });
    });
    */
}

void WLANmaus::broadcastLocoInfo(const Decoder& decoder)
{
  const uint16_t key = locoInfoKey(decoder.address, decoder.longAddress);
  const z21_lan_x_loco_info message(decoder);

//logDebug("z21_lan_x_loco_info.speedAndDirection=" + std::to_string(message.speedAndDirection));

  for(auto it : m_clients)
    if(it.second.broadcastFlags & Z21::PowerLocoTurnout)
      if(it.second.locoInfo.count(key))
        sendTo(message, it.first);
}
