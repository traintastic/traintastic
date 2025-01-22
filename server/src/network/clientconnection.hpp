/**
 * server/src/network/clientconnection.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_NETWORK_CLIENTCONNECTION_HPP
#define TRAINTASTIC_SERVER_NETWORK_CLIENTCONNECTION_HPP

#include <memory>
#include <queue>
#include <boost/asio.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include "../core/objectptr.hpp"
#include <traintastic/network/message.hpp>

class Server;
class Session;

class ClientConnection : public std::enable_shared_from_this<ClientConnection>
{
  friend class Session;

  protected:
    using ObjectHandle = uint32_t;

    Server& m_server;
    std::shared_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> m_ws;
    boost::beast::flat_buffer m_readBuffer;
    std::mutex m_writeQueueMutex;
    std::queue<std::unique_ptr<Message>> m_writeQueue;
    bool m_authenticated;
    std::shared_ptr<Session> m_session;

    void doRead();
    void doWrite();

    void processMessage(const std::shared_ptr<Message> message);
    void sendMessage(std::unique_ptr<Message> message);

    void connectionLost();

  public:
    const std::string id;

    ClientConnection(Server& server, std::shared_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> ws, std::string id_);
    virtual ~ClientConnection();

    void start();

    void disconnect();
};

#endif
