/**
 * hardware/commandstation/z21.cpp
 *
 * This file is part of the traintastic source code
 *
 * Copyright (C) 2019-2020 Reinder Feenstra <reinderfeenstra@gmail.com>
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

#include "z21.hpp"
#include "../../core/traintastic.hpp"
#include "../../world/world.hpp"
#include "../../core/eventloop.hpp"
#include "../decoder/decoderchangeflags.hpp"
#include "../protocol/xpressnet.hpp"

#include "../protocol/z21.hpp"
#include "../../utils/to_hex.hpp"









namespace Hardware::CommandStation {



#define SET_ADDRESS \
  if(decoder.longAddress) \
  { \
    cmd.addressHigh = 0xc0 | (decoder.address >> 8); \
    cmd.addressLow = decoder.address & 0xff; \
  } \
  else \
  { \
    cmd.addressHigh = 0; \
    cmd.addressLow = decoder.address; \
  }






Z21::Z21(const std::weak_ptr<World>& world, std::string_view _id) :
  CommandStation(world, _id),
  m_socket{Traintastic::instance->ioContext()},
  hostname{this, "hostname", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  loconet{this, "loconet", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  port{this, "port", 21105, PropertyFlags::ReadWrite | PropertyFlags::Store},
  serialNumber{this, "serial_number", "", PropertyFlags::ReadOnly},
  hardwareType{this, "hardware_type", "", PropertyFlags::ReadOnly},
  mainCurrent{this, "main_current", std::numeric_limits<float>::quiet_NaN(), PropertyFlags::ReadOnly},
  progCurrent{this, "prog_current", std::numeric_limits<float>::quiet_NaN(), PropertyFlags::ReadOnly},
  filteredMainCurrent{this, "filtered_main_current", std::numeric_limits<float>::quiet_NaN(), PropertyFlags::ReadOnly},
  temperature{this, "temperature", std::numeric_limits<float>::quiet_NaN(), PropertyFlags::ReadOnly},
  supplyVoltage{this, "supply_voltage", std::numeric_limits<float>::quiet_NaN(), PropertyFlags::ReadOnly},
  vccVoltage{this, "vcc_voltage", std::numeric_limits<float>::quiet_NaN(), PropertyFlags::ReadOnly},
  firmwareVersion{this, "firmware_version", "", PropertyFlags::ReadOnly},
  shortCircuit{this, "short_circuit", false, PropertyFlags::ReadOnly},
  programmingModeActive{this, "programming_mode_active", false, PropertyFlags::ReadOnly},
  highTemperature{this, "high_temperature", false, PropertyFlags::ReadOnly},
  powerLost{this, "power_lost", false, PropertyFlags::ReadOnly},
  shortCircutInternal{this, "short_circut_internal", false, PropertyFlags::ReadOnly},
  shortCircutExternal{this, "short_circut_external", false, PropertyFlags::ReadOnly}
{
  name = "Z21";
  loconet.setValueInternal(std::make_shared<::Protocol::LocoNet::LocoNet>(*this, loconet.name(),
    [/*this*/](const ::Protocol::LocoNet::Message& /*msg*/)
    {
      return false;
    }));

  m_interfaceItems.insertBefore(hostname, notes)
    .addAttributeEnabled(true);
  m_interfaceItems.insertBefore(port, notes)
    .addAttributeEnabled(true);
  m_interfaceItems.insertBefore(loconet, notes);
  m_interfaceItems.insertBefore(serialNumber, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(hardwareType, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(firmwareVersion, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(mainCurrent, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(progCurrent, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(filteredMainCurrent, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(temperature, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(supplyVoltage, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(vccVoltage, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(shortCircuit, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(programmingModeActive, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(highTemperature, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(powerLost, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(shortCircutInternal, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(shortCircutExternal, notes)
    .addAttributeCategory(Category::Info);
}

void Z21::emergencyStopChanged(bool value)
{
  if(online && value)
    send(z21_lan_x_set_stop());
}

void Z21::trackVoltageOffChanged(bool value)
{
  if(online)
  {
    if(value)
      send(z21_lan_x_set_track_power_off());
    else
      send(z21_lan_x_set_track_power_on());
  }
}

/*
bool Z21::isDecoderSupported(Decoder& decoder) const
{
  return
    decoder.protocol == DecoderProtocol::DCC &&
    decoder.address >= 1 &&
    decoder.address <= (decoder.longAddress ? 9999 : 127);
}
*/




void Z21::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Direction | DecoderChangeFlags::SpeedStep | DecoderChangeFlags::SpeedSteps))
  {
    z21_lan_x_set_loco_drive cmd;
    cmd.dataLen = sizeof(cmd);
    cmd.header = Z21_LAN_X;
    SET_ADDRESS;

    assert(decoder.speedStep <= decoder.speedSteps);
    switch(decoder.speedSteps)
    {
      case 14:
        cmd.db0 = 0x10;
        if(decoder.emergencyStop)
          cmd.speedAndDirection = 0x01;
        else if(decoder.speedStep > 0)
          cmd.speedAndDirection = decoder.speedStep + 1;
        break;

      case 28:
        cmd.db0 = 0x12;
        if(decoder.emergencyStop)
          cmd.speedAndDirection = 0x01;
        else if(decoder.speedStep > 0)
        {
          const uint8_t speedStep = decoder.speedStep + 1;
          cmd.speedAndDirection = ((speedStep & 0x01) << 4) | (speedStep >> 1);
        }
        break;

      case 126:
        cmd.db0 = 0x13;
        if(decoder.emergencyStop)
          cmd.speedAndDirection = 0x01;
        else if(decoder.speedStep > 0)
          cmd.speedAndDirection = decoder.speedStep + 1;
        break;

      default:
        return;
    }

    if(decoder.direction.value() == Direction::Forward)
      cmd.speedAndDirection |= 0x80;

    cmd.checksum = Protocol::XpressNet::calcChecksum(&cmd.xheader);
    send(&cmd);
  }
  else if(has(changes, DecoderChangeFlags::FunctionValue))
  {
    if(functionNumber <= 28)
    {
      if(const auto& f = decoder.getFunction(functionNumber))
      {
        z21_lan_x_set_loco_function cmd;
        cmd.dataLen = sizeof(cmd);
        cmd.header = Z21_LAN_X;
        SET_ADDRESS;
        cmd.db3 = (f->value ? 0x40 : 0x00) | static_cast<uint8_t>(functionNumber);
        cmd.checksum = Protocol::XpressNet::calcChecksum(&cmd.xheader);
        send(&cmd);
      }
    }
  }
}

bool Z21::setOnline(bool& value)
{
  if(!m_socket.is_open() && value)
  {
    boost::system::error_code ec;

    m_remoteEndpoint.port(port);
    m_remoteEndpoint.address(boost::asio::ip::make_address(hostname, ec));
    if(ec)
    {
      logError("make_address: " + ec.message());
      return false;
    }

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

    send(z21_lan_set_broadcastflags(0x07000000 | /*0x00010000 |*/ 0x00000100 | 0x00000001));

    // try to communicate with Z21

    send(Protocol::Z21::LanGetSerialNumber());
    send(z21_lan_get_hwinfo());
    send(z21_lan_get_broadcastflags());
    send(z21_lan_systemstate_getdata());
    for(auto& decoder : *decoders)
      send(z21_lan_x_get_loco_info(decoder->address, decoder->longAddress));

    hostname.setAttributeEnabled(false);
    port.setAttributeEnabled(false);
  }
  else if(m_socket.is_open() && !value)
  {
    send(z21_lan_logoff());

    serialNumber.setValueInternal("");
    hardwareType.setValueInternal("");
    firmwareVersion.setValueInternal("");

    hostname.setAttributeEnabled(true);
    port.setAttributeEnabled(true);

    m_socket.close();
  }
  return true;
}

void Z21::receive()
{
  m_socket.async_receive_from(boost::asio::buffer(m_receiveBuffer), m_receiveEndpoint,
    [this](const boost::system::error_code& ec, std::size_t bytesReceived)
    {
      if(!ec)
      {
        if((bytesReceived >= sizeof(z21_lan_header)))
        {
          bool unknownMessage = false;
          const Protocol::Z21::Message* message = reinterpret_cast<const Protocol::Z21::Message*>(m_receiveBuffer.data());
          const z21_lan_header* cmd = reinterpret_cast<const z21_lan_header*>(m_receiveBuffer.data());
          switch(cmd->header)
          {
            case Z21_LAN_GET_SERIAL_NUMBER:
              if(message->dataLen() == sizeof(Protocol::Z21::LanGetSerialNumberReply))
              {
                EventLoop::call(
                  [this, value=std::to_string(static_cast<const Protocol::Z21::LanGetSerialNumberReply*>(message)->serialNumber())]()
                  {
                    serialNumber.setValueInternal(value);
                  });
              }
              else
                unknownMessage = true;
              break;

            case Z21_LAN_GET_HWINFO:
            {
              const z21_lan_get_hwinfo_reply* reply = static_cast<const z21_lan_get_hwinfo_reply*>(cmd);

              std::string hwType;
              switch(reply->hardwareType)
              {
                case Z21_HWT_Z21_OLD:
                  hwType = "Black Z21 (hardware variant from 2012)";
                  break;
                case Z21_HWT_Z21_NEW:
                  hwType = "Black Z21 (hardware variant from 2013)";
                  break;
                case Z21_HWT_SMARTRAIL:
                  hwType = "SmartRail (from 2012)";
                  break;
                case Z21_HWT_Z21_SMALL:
                  hwType = "White Z21 (starter set variant from 2013)";
                  break;
                case Z21_HWT_Z21_START :
                  hwType = "Z21 start (starter set variant from 2016)";
                  break;
                default:
                  hwType = "0x" + to_hex(reply->hardwareType);
                  break;
              }

              const std::string fwVersion = std::to_string((reply->firmwareVersion >> 8) & 0xFF) + "." + std::to_string(reply->firmwareVersion & 0xFF);

              EventLoop::call(
                [this, hwType, fwVersion]()
                {
                  hardwareType.setValueInternal(hwType);
                  firmwareVersion.setValueInternal(fwVersion);
                });
              break;
            }
            case Z21_LAN_X:
            {
              // TODO check XOR
              const uint8_t xheader = static_cast<const z21_lan_x*>(cmd)->xheader;
              switch(xheader)
              {
                case Z21_LAN_X_LOCO_INFO:
                {
                  const z21_lan_x_loco_info* info = static_cast<const z21_lan_x_loco_info*>(cmd);
                  const uint16_t address = (static_cast<uint16_t>(info->addressHigh) << 8) | info->addressLow;
                  const uint8_t speedStepMode = info->db2 & 0x07;
                  const Direction direction = (info->speedAndDirection & 0x80) ? Direction::Forward : Direction::Reverse;
                  const uint8_t speedStep = info->speedAndDirection & 0x7f;
                  const uint32_t functions =
                    ((info->db4 & 0x10) >> 4) |
                    ((info->db4 & 0x0f) << 1) |
                    (static_cast<uint32_t>(info->f5f12) << 5) |
                    (static_cast<uint32_t>(info->f13f20) << 13) |
                    (static_cast<uint32_t>(info->f21f28) << 21);

                  EventLoop::call(
                    [this, address, speedStepMode, direction, speedStep, functions]()
                    {
                      const std::shared_ptr<Decoder>& decoder = getDecoder(DecoderProtocol::DCC, address & 0x3fff, address & 0xc000);
                      if(decoder)
                      {
                        decoder->direction = direction;
                        if((speedStepMode == 0 && decoder->speedSteps == 14) ||
                           (speedStepMode == 2 && decoder->speedSteps == 28) ||
                           (speedStepMode == 4 && decoder->speedSteps == 126))
                          decoder->speedStep.setValueInternal(speedStep);

                        for(auto& function : *decoder->functions)
                        {
                          const uint8_t number = function->number;
                          if(number <= 28)
                            function->value.setValueInternal(functions & (1 << number));
                        }
                      }
                    });
                  break;
                }
                case Z21_LAN_X_BC:
                {

                  break;
                }
                case Z21_LAN_X_BC_STOPPED:
                  EventLoop::call(
                    [this]()
                    {
                      emergencyStop.setValueInternal(true);
                    });
                  break;

                default:
                  EventLoop::call([this, xheader](){ logDebug("unknown xheader 0x" + to_hex(xheader)); });
                  break;
              }
              break;
            }
            case Z21_LAN_SYSTEMSTATE_DATACHANGED:
            {
              const z21_lan_systemstate_datachanged state = *reinterpret_cast<const z21_lan_systemstate_datachanged*>(m_receiveBuffer.data());
              /*EventLoop::call(
                [this, state]()
                {
                  mainCurrent.setValueInternal(state.mainCurrent / 1e3); //!< Current on the main track in mA
                  progCurrent.setValueInternal(state.progCurrent / 1e3); //!< Current on programming track in mA;
                  filteredMainCurrent.setValueInternal(state.filteredMainCurrent / 1e3); //!< Smoothed current on the main track in mA
                  temperature.setValueInternal(state.temperature); //!< Command station internal temperature in °C
                  supplyVoltage.setValueInternal(state.supplyVoltage / 1e3); //!< Supply voltage in mV
                  vccVoltage.setValueInternal(state.vccVoltage / 1e3); //!< Internal voltage, identical to track voltage in mV
                  emergencyStop.setValueInternal(state.centralState & Z21_CENTRALSTATE_EMERGENCYSTOP);
                  trackVoltageOff.setValueInternal(state.centralState & Z21_CENTRALSTATE_TRACKVOLTAGEOFF);
                  shortCircuit.setValueInternal(state.centralState & Z21_CENTRALSTATE_SHORTCIRCUIT);
                  programmingModeActive.setValueInternal(state.centralState & Z21_CENTRALSTATE_PROGRAMMINGMODEACTIVE);
                  highTemperature.setValueInternal(state.centralStateEx & Z21_CENTRALSTATEEX_HIGHTEMPERATURE);
                  powerLost.setValueInternal(state.centralStateEx & Z21_CENTRALSTATEEX_POWERLOST);
                  shortCircutInternal.setValueInternal(state.centralStateEx & Z21_CENTRALSTATEEX_SHORTCIRCUITEXTERNAL);
                  shortCircutExternal.setValueInternal(state.centralStateEx & Z21_CENTRALSTATEEX_SHORTCIRCUITINTERNAL);
                });*/
              break;
            }
            case Z21_LAN_LOCONET_Z21_RX:
            //case Z21_LAN_LOCONET_Z21_TX:
            //case Z21_LAN_LOCONET_Z21_LAN:
              loconet->receive(*reinterpret_cast<const ::Protocol::LocoNet::Message*>(m_receiveBuffer.data() + sizeof(z21_lan_header)));
              break;
/*

              using LocoNet = Protocol::LocoNet;

              const LocoNet::Header* message = reinterpret_cast<const LocoNet::Header*>(m_receiveBuffer.data() + sizeof(z21_lan_header));

              switch(message->opCode)
              {
                case LocoNet::OPC_INPUT_REP:
                {
                  const LocoNet::InputRep* inputRep = static_cast<const LocoNet::InputRep*>(message);

                  //if(debugEnabled)
                  {
                    const std::string message = "loconet rx OPC_INPUT_REP:"
                      " address=" + std::to_string(inputRep->address()) +
                      " input="  + (inputRep->isAuxInput() ? "aux" : "switch") +
                      " value=" + (inputRep->value() ? "high" : "low");
                    EventLoop::call([this, message](){ logDebug(id, message); });
                  }


                  break;
                }


                default:
                  //if(debugEnabled)
                  {
                    std::string message = "unknown loconet message: ";
                    for(int i = 4; i < cmd->dataLen; i++)
                      message += to_hex(reinterpret_cast<const uint8_t*>(cmd)[i]);
                    EventLoop::call([this, message](){ logDebug(id, message); });
                  }
                  break;
              }


  */

            default:
              //if(debugEnabled)
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
              break;
          }
        }
        receive();
      }
      else
        EventLoop::call([this, ec](){ logError("socket.async_receive_from: " + ec.message()); });
    });
}

void Z21::send(const Protocol::Z21::Message& message)
{
  // TODO async

    // TODO: add to queue, send async

  boost::system::error_code ec;
  m_socket.send_to(boost::asio::buffer(&message, message.dataLen()), m_remoteEndpoint, 0, ec);
  if(ec)
     logError("socket.send_to: " + ec.message());
/*
  m_socket.send_to(boost::asio:buffer(&message, message.dataLen()), 0, m_remoteEndpoint);,
    [this](const boost::system::error_code& ec, std::size_t)
    {
      if(ec)
         EventLoop::call([this, ec](){ logError(id, "socket.async_send_to: " + ec.message()); });
    });*/
}

void Z21::send(const z21_lan_header* data)
{
  logDebug("z21_lan_header->dataLen = " + std::to_string(data->dataLen));

  boost::system::error_code ec;
  m_socket.send_to(boost::asio::buffer(data, data->dataLen), m_remoteEndpoint, 0, ec);
  if(ec)
     logError("socket.send_to: " + ec.message());


  //m_socket.send_to(boost::asio::buffer(data, data->dataLen), 0, m_remoteEndpoint);
  /*,
    [this](const boost::system::error_code& ec, std::size_t)
    {
      if(ec)
         EventLoop::call([this, ec](){ logError(id, "socket.async_send_to: " + ec.message()); });
    });*/
}

}
