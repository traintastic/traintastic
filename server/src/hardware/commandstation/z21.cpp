/**
 * hardware/commandstation/z21.cpp
 *
 * This file is part of the traintastic source code
 *
 * Copyright (C) 2019 Reinder Feenstra <reinderfeenstra@gmail.com>
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
#include "../../core/world.hpp"
#include "../../core/eventloop.hpp"
#include "../decoder/decoderchangeflags.hpp"
#include "protocol/xpressnet.hpp"
#include "protocol/z21.hpp"
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






Z21::Z21(const std::weak_ptr<World>& world, const std::string& _id) :
  CommandStation(world, _id),
  m_socket{Traintastic::instance->ioContext()},
  hostname{this, "hostname", "", PropertyFlags::AccessWCC},
  port{this, "port", 21105, PropertyFlags::AccessWCC},
  serialNumber{this, "serial_number", "", PropertyFlags::AccessRRR},
  hardwareType{this, "hardware_type", "", PropertyFlags::AccessRRR},
  firmwareVersion{this, "firmware_version", "", PropertyFlags::AccessRRR},
  emergencyStop{this, "emergency_stop", false, PropertyFlags::TODO,
    [this](bool value)
    {
      if(online && value)
        send(z21_lan_x_set_stop());
    }},
  trackVoltageOff{this, "track_voltage_off", false, PropertyFlags::TODO,
    [this](bool value)
    {
      if(online)
      {
        if(value)
          send(z21_lan_x_set_track_power_off());
        else
          send(z21_lan_x_set_track_power_on());
      }
    }}
{
  name = "Z21";

  m_interfaceItems.insertBefore(hostname, notes)
    .addAttributeEnabled(true);
  m_interfaceItems.insertBefore(port, notes)
    .addAttributeEnabled(true);
  m_interfaceItems.insertBefore(serialNumber, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(hardwareType, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(firmwareVersion, notes)
    .addAttributeCategory(Category::Info);
  m_interfaceItems.insertBefore(emergencyStop, notes)
    .addAttributeEnabled(false);
  m_interfaceItems.insertBefore(trackVoltageOff, notes)
    .addAttributeEnabled(false);
}

bool Z21::isDecoderSupported(Decoder& decoder) const
{
  return
    decoder.protocol == DecoderProtocol::DCC &&
    decoder.address >= 1 &&
    decoder.address <= (decoder.longAddress ? 9999 : 127);
}

void Z21::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  using Protocol::XpressNet;

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

    if(decoder.direction == Direction::Forward)
      cmd.speedAndDirection |= 0x80;

    cmd.checksum = XpressNet::calcChecksum(&cmd.xheader);
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
        cmd.checksum = XpressNet::calcChecksum(&cmd.xheader);
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
      Traintastic::instance->console->error(id, "make_address: " + ec.message());
      return false;
    }

    if(m_socket.open(boost::asio::ip::udp::v4(), ec))
    {
      Traintastic::instance->console->error(id, "socket.open: " + ec.message());
      return false;
    }
    else if(m_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), port), ec))
    {
      m_socket.close();
      Traintastic::instance->console->error(id, "socket.bind: " + ec.message());
      return false;
    }

    receive();

    send(z21_lan_set_broadcastflags(/*0x00010000 |*/ 0x00000100 | 0x00000001));

    // try to communicate with Z21
    send(z21_lan_get_broadcastflags());
    send(z21_lan_get_serial_number());
    send(z21_lan_get_hwinfo());
    send(z21_lan_systemstate_getdata());
/*
    for(auto& decoder : decoders)
    {
      z21_lan_x_get_loco_info cmd;
      send(cmd);
    }
*/
    hostname.setAttributeEnabled(false);
    port.setAttributeEnabled(false);
    emergencyStop.setAttributeEnabled(true);
    trackVoltageOff.setAttributeEnabled(true);
  }
  else if(m_socket.is_open() && !value)
  {
    send(z21_lan_logoff());

    serialNumber = "";
    hardwareType = "";
    firmwareVersion = "";

    hostname.setAttributeEnabled(true);
    port.setAttributeEnabled(true);
    emergencyStop.setAttributeEnabled(false);
    trackVoltageOff.setAttributeEnabled(false);

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
          const z21_lan_header* cmd = reinterpret_cast<const z21_lan_header*>(m_receiveBuffer.data());
          switch(cmd->header)
          {
            case Z21_LAN_GET_SERIAL_NUMBER:
            {
              EventLoop::call(
                [this, value=std::to_string(static_cast<const z21_lan_get_serial_number_reply*>(cmd)->serialNumber)]()
                {
                  serialNumber.setValueInternal(value);
                });
              break;
            }
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
                  EventLoop::call([this, xheader](){ Traintastic::instance->console->debug(id, "unknown xheader 0x" + to_hex(xheader)); });
                  break;
              }
              break;
            }
            case Z21_LAN_SYSTEMSTATE_DATACHANGED:
            {
              const z21_lan_systemstate_datachanged state = *reinterpret_cast<const z21_lan_systemstate_datachanged*>(m_receiveBuffer.data());
              EventLoop::call(
                [this, state]()
                {
                  emergencyStop.setValueInternal(state.centralState & Z21_CENTRALSTATE_EMERGENCYSTOP);
                  trackVoltageOff.setValueInternal(state.centralState & Z21_CENTRALSTATE_TRACKVOLTAGEOFF);
                });
              break;
            }
            default:
              EventLoop::call([this, header=cmd->header](){ Traintastic::instance->console->debug(id, "unknown header 0x" + to_hex(header)); });
              break;
          }
        }
        receive();
      }
      else
        EventLoop::call([this, ec](){ Traintastic::instance->console->error(id, "socket.async_receive_from: " + ec.message()); });
    });
}

void Z21::send(const z21_lan_header* data)
{
  Traintastic::instance->console->debug(id, "z21_lan_header->dataLen = " + std::to_string(data->dataLen));

  m_socket.async_send_to(boost::asio::buffer(data, data->dataLen), m_remoteEndpoint,
    [this](const boost::system::error_code& ec, std::size_t)
    {
      if(ec)
         EventLoop::call([this, ec](){ Traintastic::instance->console->error(id, "socket.async_send_to: " + ec.message()); });
    });
}

}
