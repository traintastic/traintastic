/**
 * server/src/network/server.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_NETWORK_SERVER_HPP
#define TRAINTASTIC_SERVER_NETWORK_SERVER_HPP

#include <memory>
#include <array>
#include <list>
#include <thread>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

class Client;
class Message;

class Server : public std::enable_shared_from_this<Server>
{
  friend class Client;

  private:
    static constexpr std::string_view id{"server"};
    boost::asio::io_context m_ioContext;
    std::thread m_thread;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socketTCP;
    boost::asio::ip::udp::socket m_socketUDP;
    std::array<char, 8> m_udpBuffer;
    boost::asio::ip::udp::endpoint m_remoteEndpoint;
    bool m_localhostOnly;
    std::list<std::shared_ptr<Client>> m_clients;

    void doReceive();
    std::unique_ptr<Message> processMessage(const Message& message);
    void doAccept();

    void clientGone(const std::shared_ptr<Client>& client);

  public:
    static constexpr uint16_t defaultPort = 5740; //!< unoffical, not (yet) assigned by IANA

    Server();
    ~Server();

#ifndef NDEBUG
    inline auto threadId() const { return m_thread.get_id(); }
#endif

    bool start(bool localhostOnly, uint16_t port, bool discoverable);
    void stop();
};

#endif
