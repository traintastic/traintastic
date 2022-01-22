/**
 * server/src/core/traintastic.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "traintastic.hpp"
#include <fstream>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <version.hpp>
#include <traintastic/codename.hpp>
#include <traintastic/utils/str.hpp>
#include "eventloop.hpp"
#include "settings.hpp"
#include "client.hpp"
#include "attributes.hpp"
#include "../world/world.hpp"
#include "../world/worldlist.hpp"
#include "../world/worldloader.hpp"
#include "../log/log.hpp"

using nlohmann::json;

std::shared_ptr<Traintastic> Traintastic::instance;

Traintastic::Traintastic(const std::filesystem::path& dataDir) :
  m_restart{false},
  m_dataDir{std::filesystem::absolute(dataDir)},
  m_ioContext{},
  m_acceptor{m_ioContext},
  m_socketTCP{m_ioContext},
  m_socketUDP{m_ioContext},
  settings{this, "settings", nullptr, PropertyFlags::ReadWrite/*ReadOnly*/},
  world{this, "world", nullptr, PropertyFlags::ReadWrite},
  worldList{this, "world_list", nullptr, PropertyFlags::ReadWrite/*ReadOnly*/},
  newWorld{*this, "new_world",
    [this]()
    {
      world = World::create();
      Log::log(*this, LogMessage::N1002_CREATED_NEW_WORLD);
      world->edit = true;
      settings->lastWorld = "";
    }},
  loadWorld{*this, "load_world",
    [this](const std::string& _uuid)
    {
      boost::uuids::uuid uuid;
      try
      {
        uuid = boost::uuids::string_generator()(_uuid);
      }
      catch(const std::exception&)
      {
        uuid = boost::uuids::nil_generator()();
        Log::log(*this, LogMessage::E1001_INVALID_WORLD_UUID_X, _uuid);
      }

      if(!uuid.is_nil())
        loadWorldUUID(uuid);
    }},
  restart{*this, "restart",
    [this]()
    {
      if(Attributes::getEnabled(restart))
      {
        m_restart = true;
        exit();
      }
    }},
  shutdown{*this, "shutdown",
    [this]()
    {
      if(Attributes::getEnabled(shutdown))
        exit();
    }}
{
  if(!std::filesystem::is_directory(m_dataDir))
    std::filesystem::create_directories(m_dataDir);

  m_interfaceItems.add(settings);
  m_interfaceItems.add(world);
  m_interfaceItems.add(worldList);
  m_interfaceItems.add(newWorld);
  m_interfaceItems.add(loadWorld);
  Attributes::addEnabled(restart, false);
  m_interfaceItems.add(restart);
  Attributes::addEnabled(shutdown, false);
  m_interfaceItems.add(shutdown);
}

Traintastic::~Traintastic()
{
  assert(m_ioContext.stopped());
}

Traintastic::RunStatus Traintastic::run()
{
  Log::log(*this, LogMessage::I1001_TRAINTASTIC_VX_X, std::string_view{TRAINTASTIC_VERSION}, std::string_view{TRAINTASTIC_CODENAME});

  settings = std::make_shared<Settings>(m_dataDir);
  Attributes::setEnabled(restart, settings->allowClientServerRestart);
  Attributes::setEnabled(shutdown, settings->allowClientServerShutdown);

  worldList = std::make_shared<WorldList>(worldDir());

  if(settings->loadLastWorldOnStartup && !settings->lastWorld.value().empty())
    loadWorld(settings->lastWorld.value());

  if(!start())
    return ExitFailure;

  auto work = std::make_shared<boost::asio::io_service::work>(m_ioContext);
  m_ioContext.run();

  return m_restart ? Restart : ExitSuccess;
}

void Traintastic::exit()
{
  if(m_restart)
    Log::log(*this, LogMessage::N1003_RESTARTING);
  else
    Log::log(*this, LogMessage::N1004_SHUTTING_DOWN);

  if(settings->autoSaveWorldOnExit && world)
    world->save();

  stop();

  m_ioContext.stop();
}

bool Traintastic::start()
{
  boost::system::error_code ec;
  boost::asio::ip::tcp::endpoint endpoint(settings->localhostOnly ? boost::asio::ip::address_v4::loopback() : boost::asio::ip::address_v4::any(), settings->port);

  m_acceptor.open(endpoint.protocol(), ec);
  if(ec)
  {
    Log::log(*this, LogMessage::F1001_OPENING_TCP_SOCKET_FAILED_X, ec.message());
    return false;
  }

  m_acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
  if(ec)
  {
    Log::log(*this, LogMessage::F1002_TCP_SOCKET_ADDRESS_REUSE_FAILED_X, ec.message());
    return false;
  }

  m_acceptor.bind(endpoint, ec);
  if(ec)
  {
    Log::log(*this, LogMessage::F1003_BINDING_TCP_SOCKET_FAILED_X, ec.message());
    return false;
  }

  m_acceptor.listen(5, ec);
  if(ec)
  {
    Log::log(*this, LogMessage::F1004_TCP_SOCKET_LISTEN_FAILED_X, ec.message());
    return false;
  }

  if(settings->discoverable)
  {
    if(settings->port == Settings::defaultPort)
    {
      m_socketUDP.open(boost::asio::ip::udp::v4(), ec);
      if(ec)
      {
        Log::log(*this, LogMessage::F1005_OPENING_UDP_SOCKET_FAILED_X, ec.message());
        return false;
      }

      m_socketUDP.set_option(boost::asio::socket_base::reuse_address(true), ec);
      if(ec)
      {
        Log::log(*this, LogMessage::F1006_UDP_SOCKET_ADDRESS_REUSE_FAILED_X, ec.message());
        return false;
      }

      m_socketUDP.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), Settings::defaultPort), ec);
      if(ec)
      {
        Log::log(*this, LogMessage::F1007_BINDING_UDP_SOCKET_FAILED_X, ec.message());
        return false;
      }

      Log::log(*this, LogMessage::N1005_DISCOVERY_ENABLED);
      doReceive();
    }
    else
      Log::log(*this, LogMessage::W1001_DISCOVERY_DISABLED_ONLY_ALLOWED_ON_PORT_X, Settings::defaultPort);
  }
  else
    Log::log(*this, LogMessage::N1006_DISCOVERY_DISABLED);

  Log::log(*this, LogMessage::N1007_LISTENING_AT_X_X, m_acceptor.local_endpoint().address().to_string(), m_acceptor.local_endpoint().port());
  doAccept();

  return true;
}

void Traintastic::stop()
{
  for(auto& client : m_clients)
  {
    client->stop();
    client.reset();
  }

  boost::system::error_code ec;
  m_acceptor.cancel(ec);
  if(ec)
    Log::log(*this, LogMessage::E1008_SOCKET_ACCEPTOR_CANCEL_FAILED_X, ec);
  m_acceptor.close();

  m_socketUDP.close();
}

void Traintastic::loadWorldUUID(const boost::uuids::uuid& uuid)
{
  if(const WorldList::WorldInfo* info = worldList->find(uuid))
    loadWorldPath(info->path);
  else
    Log::log(*this, LogMessage::E1002_WORLD_X_DOESNT_EXIST, to_string(uuid));
}

void Traintastic::loadWorldPath(const std::filesystem::path& path)
{
  try
  {
    world = WorldLoader(path).world();
    settings->lastWorld = world->uuid.value();
  }
  catch(const std::exception& e)
  {
    Log::log(*this, LogMessage::C1001_LOADING_WORLD_FAILED_X, e.what());
  }
}

void Traintastic::clientGone(const std::shared_ptr<Client>& client)
{
  m_clients.erase(std::find(m_clients.begin(), m_clients.end(), client));
}

void Traintastic::doReceive()
{
  m_socketUDP.async_receive_from(boost::asio::buffer(m_udpBuffer), m_remoteEndpoint,
    [this](const boost::system::error_code& ec, std::size_t bytesReceived)
    {
      if(!ec)
      {
        if(bytesReceived == sizeof(Message::Header))
        {
          Message message(*reinterpret_cast<Message::Header*>(m_udpBuffer.data()));

          if(!settings->localhostOnly || m_remoteEndpoint.address().is_loopback())
          {
            if(message.dataSize() == 0)
            {
              std::unique_ptr<Message> response = processMessage(message);
              if(response)
              {
                m_socketUDP.async_send_to(boost::asio::buffer(**response, response->size()), m_remoteEndpoint,
                  [this](const boost::system::error_code& /*ec*/, std::size_t /*bytesTransferred*/)
                  {
                    doReceive();
                  });
                return;
              }
            }
          }
        }
        doReceive();
      }
      else
        Log::log(*this, LogMessage::E1003_UDP_RECEIVE_ERROR_X, ec.message());
    });
}

std::unique_ptr<Message> Traintastic::processMessage(const Message& message)
{
  if(message.command() == Message::Command::Discover && message.isRequest())
  {
    std::unique_ptr<Message> response = Message::newResponse(message.command(), message.requestId());
    response->write(boost::asio::ip::host_name());
    return response;
  }

  return nullptr;
}

void Traintastic::doAccept()
{
  m_acceptor.async_accept(m_socketTCP,
    [this](boost::system::error_code ec)
    {
      EventLoop::call(
        [this, ec]()
        {
          if(!ec)
          {
            try
            {
              std::shared_ptr<Client> client = std::make_shared<Client>(*this, "client[" + m_socketTCP.remote_endpoint().address().to_string() + ":" + std::to_string(m_socketTCP.remote_endpoint().port()) + "]", std::move(m_socketTCP));
              client->start();
              m_clients.push_back(client);
              doAccept();
            }
            catch(const std::exception& e)
            {
              Log::log(*this, LogMessage::C1002_CREATING_CLIENT_FAILED_X, e.what());
            }
          }
          else
            Log::log(*this, LogMessage::E1004_TCP_ACCEPT_ERROR_X, ec.message());
        });
    });
}
