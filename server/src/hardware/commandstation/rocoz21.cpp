/**
 * server/src/hardware/commandstation/rocoz21.cpp
 *
 * This file is part of the traintastic source code
 *
 * Copyright (C) 2019-2021 Reinder Feenstra <reinderfeenstra@gmail.com>
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

#include "rocoz21.hpp"
#include "../../core/traintastic.hpp"
#include "../../world/world.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/attributes.hpp"
#include "../decoder/decoderchangeflags.hpp"
#include "../protocol/xpressnet/messages.hpp"
#include "../protocol/z21/messages.hpp"
#include "../../utils/tohex.hpp"













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






RocoZ21::RocoZ21(const std::weak_ptr<World>& world, std::string_view _id) :
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
  loconet.setValueInternal(LocoNet::LocoNet::create(*this, loconet.name(),
    [/*this*/](const ::LocoNet::Message& /*msg*/)
    {
      return false;
    }));

  Attributes::addEnabled(hostname, true);
  m_interfaceItems.insertBefore(hostname, notes);
  Attributes::addEnabled(port, true);
  m_interfaceItems.insertBefore(port, notes);
  m_interfaceItems.insertBefore(loconet, notes);
  Attributes::addCategory(serialNumber, Category::Info);
  m_interfaceItems.insertBefore(serialNumber, notes);
  Attributes::addCategory(hardwareType, Category::Info);
  m_interfaceItems.insertBefore(hardwareType, notes);
  Attributes::addCategory(firmwareVersion, Category::Info);
  m_interfaceItems.insertBefore(firmwareVersion, notes);
  Attributes::addCategory(mainCurrent, Category::Info);
  m_interfaceItems.insertBefore(mainCurrent, notes);
  Attributes::addCategory(progCurrent, Category::Info);
  m_interfaceItems.insertBefore(progCurrent, notes);
  Attributes::addCategory(filteredMainCurrent, Category::Info);
  m_interfaceItems.insertBefore(filteredMainCurrent, notes);
  Attributes::addCategory(temperature, Category::Info);
  m_interfaceItems.insertBefore(temperature, notes);
  Attributes::addCategory(firmwareVersion, Category::Info);
  m_interfaceItems.insertBefore(supplyVoltage, notes);
  Attributes::addCategory(vccVoltage, Category::Info);
  m_interfaceItems.insertBefore(vccVoltage, notes);
  Attributes::addCategory(firmwareVersion, Category::Info);
  m_interfaceItems.insertBefore(shortCircuit, notes);
  Attributes::addCategory(programmingModeActive, Category::Info);
  m_interfaceItems.insertBefore(programmingModeActive, notes);
  Attributes::addCategory(highTemperature, Category::Info);
  m_interfaceItems.insertBefore(highTemperature, notes);
  Attributes::addCategory(powerLost, Category::Info);
  m_interfaceItems.insertBefore(powerLost, notes);
  Attributes::addCategory(shortCircutInternal, Category::Info);
  m_interfaceItems.insertBefore(shortCircutInternal, notes);
  Attributes::addCategory(shortCircutExternal, Category::Info);
  m_interfaceItems.insertBefore(shortCircutExternal, notes);
}

void RocoZ21::emergencyStopChanged(bool value)
{
  if(online && value)
    send(Z21::LanXSetStop());
}

void RocoZ21::powerOnChanged(bool value)
{
  if(online)
  {
    if(value)
      send(Z21::LanXSetTrackPowerOn());
    else
      send(Z21::LanXSetTrackPowerOff());
  }
}

/*
bool RocoZ21::isDecoderSupported(Decoder& decoder) const
{
  return
    decoder.protocol == DecoderProtocol::DCC &&
    decoder.address >= 1 &&
    decoder.address <= (decoder.longAddress ? 9999 : 127);
}
*/




void RocoZ21::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Direction | DecoderChangeFlags::SpeedStep | DecoderChangeFlags::SpeedSteps))
  {
    Z21::LanXSetLocoDrive cmd;
    //cmd.dataLen = sizeof(cmd);
    //cmd.header = Z21::LAN_X;
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

    cmd.checksum = XpressNet::calcChecksum(*reinterpret_cast<const XpressNet::Message*>(&cmd.xheader));
    send(cmd);
  }
  else if(has(changes, DecoderChangeFlags::FunctionValue))
  {
    if(functionNumber <= 28)
    {
      if(const auto& f = decoder.getFunction(functionNumber))
      {
        Z21::LanXSetLocoFunction cmd;
        //cmd.dataLen = sizeof(cmd);
        //cmd.header = Z21_LAN_X;
        SET_ADDRESS;
        cmd.db3 = (f->value ? 0x40 : 0x00) | static_cast<uint8_t>(functionNumber);
        cmd.checksum = XpressNet::calcChecksum(*reinterpret_cast<const XpressNet::Message*>(&cmd.xheader));
        send(cmd);
      }
    }
  }
}

bool RocoZ21::setOnline(bool& value)
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

    send(Z21::LanSetBroadcastFlags(0x07000000 | /*0x00010000 |*/ 0x00000100 | 0x00000001));

    // try to communicate with Z21

    send(Z21::LanGetSerialNumber());
    send(Z21::LanGetHardwareInfo());
    send(Z21::LanGetBroadcastFlags());
    send(Z21::LanSystemStateGetData());
    for(auto& decoder : *decoders)
      send(Z21::LanXGetLocoInfo(decoder->address, decoder->longAddress));

    hostname.setAttributeEnabled(false);
    port.setAttributeEnabled(false);
  }
  else if(m_socket.is_open() && !value)
  {
    send(Z21::LanLogoff());

    serialNumber.setValueInternal("");
    hardwareType.setValueInternal("");
    firmwareVersion.setValueInternal("");

    hostname.setAttributeEnabled(true);
    port.setAttributeEnabled(true);

    m_socket.close();
  }
  return true;
}

void RocoZ21::receive()
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
          //const z21_lan_header* cmd = reinterpret_cast<const z21_lan_header*>(m_receiveBuffer.data());
          switch(message->header())
          {
            case Z21::LAN_GET_SERIAL_NUMBER:
              if(message->dataLen() == sizeof(Z21::LanGetSerialNumberReply))
              {
                EventLoop::call(
                  [this, value=std::to_string(static_cast<const Z21::LanGetSerialNumberReply*>(message)->serialNumber())]()
                  {
                    serialNumber.setValueInternal(value);
                  });
              }
              else
                unknownMessage = true;
              break;

            case Z21::LAN_GET_HWINFO:
            {
              const Z21::LanGetHardwareInfoReply* reply = static_cast<const Z21::LanGetHardwareInfoReply*>(message);

              std::string hwType;
              switch(reply->hardwareType())
              {
                case Z21::HWT_Z21_OLD:
                  hwType = "Black Z21 (hardware variant from 2012)";
                  break;
                case Z21::HWT_Z21_NEW:
                  hwType = "Black Z21 (hardware variant from 2013)";
                  break;
                case Z21::HWT_SMARTRAIL:
                  hwType = "SmartRail (from 2012)";
                  break;
                case Z21::HWT_Z21_SMALL:
                  hwType = "White Z21 (starter set variant from 2013)";
                  break;
                case Z21::HWT_Z21_START :
                  hwType = "Z21 start (starter set variant from 2016)";
                  break;
                default:
                  hwType = "0x" + toHex(reply->hardwareType());
                  break;
              }

              const std::string fwVersion = std::to_string(reply->firmwareVersionMajor()) + "." + std::to_string(reply->firmwareVersionMinor());

              EventLoop::call(
                [this, hwType, fwVersion]()
                {
                  hardwareType.setValueInternal(hwType);
                  firmwareVersion.setValueInternal(fwVersion);
                });
              break;
            }
            case Z21::LAN_X:
            {
              // TODO check XOR
              const uint8_t xheader = static_cast<const Z21::LanX*>(message)->xheader;
              switch(xheader)
              {
                case Z21::LAN_X_LOCO_INFO:
                {
                  const Z21::LanXLocoInfo* info = static_cast<const Z21::LanXLocoInfo*>(message);
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
                case Z21::LAN_X_BC:
                {

                  break;
                }
                case Z21::LAN_X_BC_STOPPED:
                  EventLoop::call(
                    [this]()
                    {
                      emergencyStop.setValueInternal(true);
                    });
                  break;

                default:
                  EventLoop::call([this, xheader](){ logDebug("unknown xheader 0x" + toHex(xheader)); });
                  break;
              }
              break;
            }
            case Z21::LAN_SYSTEMSTATE_DATACHANGED:
            {
              const Z21::LanSystemStateDataChanged state = *reinterpret_cast<const Z21::LanSystemStateDataChanged*>(m_receiveBuffer.data());
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
            case Z21::LAN_LOCONET_Z21_RX:
            //case Z21_LAN_LOCONET_Z21_TX:
            //case Z21_LAN_LOCONET_Z21_LAN:
              loconet->receive(*reinterpret_cast<const ::LocoNet::Message*>(m_receiveBuffer.data() + sizeof(Z21::Message)));
              break;
/*

              using LocoNet = LocoNet;

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
                      message += toHex(reinterpret_cast<const uint8_t*>(cmd)[i]);
                    EventLoop::call([this, message](){ logDebug(id, message); });
                  }
                  break;
              }


  */

            default:
              //if(debugEnabled)
              {
                std::string log = "unknown message: dataLen=0x" + toHex(message->dataLen()) + ", header=0x" + toHex(message->header());
                if(message->dataLen() > 4)
                {
                  log += ", data=";
                  for(int i = sizeof(Z21::Message); i < message->dataLen(); i++)
                    log += toHex(reinterpret_cast<const uint8_t*>(message)[i]);
                }
                EventLoop::call([this, log](){ logDebug(log); });
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

void RocoZ21::send(const Z21::Message& message)
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
