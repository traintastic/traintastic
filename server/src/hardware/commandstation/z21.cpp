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
#include "../../utils/to_hex.hpp"









namespace Hardware::CommandStation {

#define Z21_HEADER_X 0x40
#define Z21_LAN_X_LOCO_INFO 0xEF

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

struct z21_lan_header
{
  uint16_t dataLen; // LE
  uint16_t header;  // LE
} __attribute__((packed));

struct z21_lan_x : z21_lan_header
{
  uint8_t xheader;
} __attribute__((packed));

struct z21_lan_x_loco_info : z21_lan_x
{
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t db2;
  uint8_t speedAndDirection;
  uint8_t db4;
  uint8_t f5f12;
  uint8_t f13f20;
  uint8_t f21f28;
} __attribute__((packed));

struct z21_lan_x_set_loco_drive : z21_lan_header
{
  uint8_t xheader = 0xe4;
  uint8_t db0;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t speedAndDirection = 0;
  uint8_t checksum;
} __attribute__((packed));
static_assert(sizeof(z21_lan_x_set_loco_drive) == 0x0a);

struct z21_lan_x_set_loco_function : z21_lan_header
{
  uint8_t xheader = 0xe4;
  uint8_t db0 = 0xf8;
  uint8_t addressHigh;
  uint8_t addressLow;
  uint8_t db3;
  uint8_t checksum;
} __attribute__((packed));
static_assert(sizeof(z21_lan_x_set_loco_function) == 0x0a);




Z21::Z21(const std::weak_ptr<World>& world, const std::string& _id) :
  CommandStation(world, _id),
  m_socket{Traintastic::instance->ioContext()},
  hostname{this, "hostname", "", PropertyFlags::AccessWCC},
  port{this, "port", 21105, PropertyFlags::AccessWCC}
{
  name = "Z21";

  m_interfaceItems.add(hostname);
  m_interfaceItems.add(port);
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
    cmd.header = Z21_HEADER_X;
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
        cmd.header = Z21_HEADER_X;
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
    // try to communicate with Z21
  }
  else if(m_socket.is_open() && !value)
  {
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
            case Z21_HEADER_X:
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
                      }
                    });
                  break;
                }
                default:
                  EventLoop::call([this, xheader](){ Traintastic::instance->console->debug(id, "unknown xheader 0x" + to_hex(xheader)); });
                  break;
              }
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
