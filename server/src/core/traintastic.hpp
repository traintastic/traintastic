/**
 * server/src/core/traintastic.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_TRAINTASTIC_HPP
#define TRAINTASTIC_SERVER_CORE_TRAINTASTIC_HPP

#include <memory>
#include <list>
#include "stdfilesystem.hpp"
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include "object.hpp"
#include "objectproperty.hpp"
#include "method.hpp"
#include "console.hpp"
//#include <enum/traintasticmode.hpp>

class Client;
class Message;
class Settings;
class World;
class WorldList;

class Traintastic : public Object
{
  friend class Client;

  private:
    const std::filesystem::path m_dataDir;
    boost::asio::io_context m_ioContext;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socketTCP;
    boost::asio::ip::udp::socket m_socketUDP;
    std::array<char, 8> m_udpBuffer;
    boost::asio::ip::udp::endpoint m_remoteEndpoint;
    std::list<std::shared_ptr<Client>> m_clients;

    bool start();
    bool stop();

    void loadWorld(const boost::uuids::uuid& uuid);
    void loadWorld(const std::filesystem::path& path);
    void saveWorld();

    void doReceive();
    std::unique_ptr<Message> processMessage(const Message& message);
    void doAccept();

  protected:
    void clientGone(const std::shared_ptr<Client>& client);

  public:
    CLASS_ID("traintastic");

    static const std::string id;
    static std::shared_ptr<Traintastic> instance;

    //Property<TraintasticMode> mode;
    ObjectProperty<Console> console;
    ObjectProperty<Settings> settings;
    ObjectProperty<World> world;
    ObjectProperty<WorldList> worldList;
    Method<void()> newWorld;

    Traintastic(const std::filesystem::path& dataDir);
    ~Traintastic() final;

    boost::asio::io_context& ioContext() { return m_ioContext; }

    //const std::string& getId() const final { return id; }

    bool run();
    void shutdown();
};

#endif
