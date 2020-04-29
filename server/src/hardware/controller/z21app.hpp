/**
 * server/src/hardware/controller/z21app.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_CONTROLLER_Z21APP_HPP
#define TRAINTASTIC_SERVER_HARDWARE_CONTROLLER_Z21APP_HPP

#include "controller.hpp"
#include <map>
#include <set>
#include <boost/asio.hpp>

struct z21_lan_header;

namespace Protocol::Z21 {
  enum BroadcastFlags : uint32_t;
  class Message;
}

namespace Hardware::Controller {

class Z21App : public Controller
{
  protected:
    struct Client
    {
      Protocol::Z21::BroadcastFlags broadcastFlags = static_cast<Protocol::Z21::BroadcastFlags>(0);
      std::set<uint16_t> locoInfo;
    };

    boost::asio::ip::udp::socket m_socket;
    boost::asio::ip::udp::endpoint m_receiveEndpoint;
    std::array<uint8_t,64> m_receiveBuffer;
    std::map<boost::asio::ip::udp::endpoint, Client> m_clients;
    Hardware::Decoder* m_blockLocoInfo;

    constexpr uint16_t locoInfoKey(uint16_t address, bool longAddress)
    {
      if(longAddress)
        return address | 0xC000;
      else
        return address;
    }

    bool setActive(bool& value) final;

    void emergencyStopChanged(bool value) final;
    void trackPowerChanged(bool value) final;
    void decoderChanged(const Hardware::Decoder& decoder, Hardware::DecoderChangeFlags, uint32_t) final;

    void receive();
    /*[[deprecated]]*/ void sendTo(const z21_lan_header& msg, const boost::asio::ip::udp::endpoint& endpoint);
    void sendTo(const Protocol::Z21::Message& message, const boost::asio::ip::udp::endpoint& endpoint);
    void broadcastLocoInfo(const Hardware::Decoder& decoder);

  public:
    CLASS_ID("hardware.controller.z21_app")
    CREATE(Z21App);

    Property<uint16_t> port;

    Z21App(const std::weak_ptr<World> world, std::string_view _id);
};

}

#endif
