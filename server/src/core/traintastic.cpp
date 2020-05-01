/**
 * server/src/core/traintastic.cpp
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

#include "traintastic.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "eventloop.hpp"
#include "settings.hpp"
#include "client.hpp"
#include "world.hpp"
#include "worldlist.hpp"
#include "worldloader.hpp"

using nlohmann::json;

const std::string Traintastic::id{Traintastic::classId};

std::shared_ptr<Traintastic> Traintastic::instance;

Traintastic::Traintastic(const std::filesystem::path& dataDir) :
  Object{},
  m_dataDir{std::filesystem::absolute(dataDir)},
  m_ioContext{},
  m_acceptor{m_ioContext},
  m_socketTCP{m_ioContext},
  m_socketUDP{m_ioContext},
  /*mode{this, "mode", TraintasticMode::Stop, PropertyFlags::ReadWrite,
    [this](const TraintasticMode& value)
    {
      assert(world);
      console->info(id, "Mode changed to <TODO> " + std::to_string((int)value));
      //world->modeChanged(value);
    },
    [this](TraintasticMode& newValue)
    {
      return
        (mode == TraintasticMode::Stop) ||
        (newValue == TraintasticMode::Stop);
    }},*/
  console{this, "console", std::make_shared<Console>(), PropertyFlags::ReadOnly},
  settings{this, "settings", nullptr, PropertyFlags::ReadWrite/*ReadOnly*/},
  world{this, "world", nullptr, PropertyFlags::ReadWrite},
  worldList{this, "world_list", nullptr, PropertyFlags::ReadWrite/*ReadOnly*/},
  newWorld{*this, "new_world",
    [this]()
    {
      world = World::create();
      console->notice(id, "Created new world");
    }}
{
  //m_interfaceItems.add(mode);
  m_interfaceItems.add(console);
  m_interfaceItems.add(settings);
  m_interfaceItems.add(world);
  m_interfaceItems.add(worldList);
  m_interfaceItems.add(newWorld);
}

Traintastic::~Traintastic()
{
  assert(m_ioContext.stopped());
}

bool Traintastic::run()
{
  settings = std::make_shared<Settings>(m_dataDir / "settings.json");
  worldList = std::make_shared<WorldList>(m_dataDir / "world");

  if(!settings->defaultWorld.value().empty())
  {
    boost::uuids::uuid uuid;
    try
    {
      uuid = boost::uuids::string_generator()(settings->defaultWorld.value());
    }
    catch(const std::exception&)
    {
      uuid = boost::uuids::nil_generator()();
      console->error(id, "Invalid default world uuid");
    }

    if(!uuid.is_nil())
      loadWorld(uuid);
  }

  if(!start())
    return false;

  auto work = std::make_shared<boost::asio::io_service::work>(m_ioContext);
  m_ioContext.run();

  return true;
}

void Traintastic::shutdown()
{
  console->notice(id, "Shutting down");

  //if(mode == TraintasticMode::Run)
  //  mode = TraintasticMode::Stop;

  if(settings->autoSaveWorldOnExit && world)
    world->save();

  m_ioContext.stop();
}

bool Traintastic::start()
{
  boost::system::error_code ec;

  m_acceptor.open(boost::asio::ip::tcp::v4(), ec);
  if(ec)
  {
    console->fatal(id, ec.message());
    return false;
  }

  m_acceptor.bind(boost::asio::ip::tcp::endpoint(settings->localhostOnly ? boost::asio::ip::address_v4::loopback() : boost::asio::ip::address_v4::any(), settings->port), ec);
  if(ec)
  {
    console->fatal(id, ec.message());
    return false;
  }

  m_acceptor.listen(5, ec);
  if(ec)
  {
    console->fatal(id, ec.message());
    return false;
  }

  if(settings->discoverable)
  {
    if(settings->port == Settings::defaultPort)
    {
      m_socketUDP.open(boost::asio::ip::udp::v4(), ec);
      if(ec)
      {
        console->fatal(id, ec.message());
        return false;
      }

      m_socketUDP.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), Settings::defaultPort), ec);
      if(ec)
      {
        console->fatal(id, "bind: " + ec.message());
        return false;
      }

      console->notice(id, "Discovery enabled");
      doReceive();
    }
    else
      console->warning(id, std::string("Discovery disabled, only allowed on port ") + std::to_string(Settings::defaultPort));
  }
  else
    console->notice(id, "Discovery disabled");

  console->notice(id, std::string("Listening at ") + m_acceptor.local_endpoint().address().to_string() + ":" + std::to_string(m_acceptor.local_endpoint().port()));
  doAccept();

  return true;
}

bool Traintastic::stop()
{
  m_acceptor.close();
  return true;
}

void Traintastic::loadWorld(const boost::uuids::uuid& uuid)
{
  if(const WorldList::WorldInfo* info = worldList->find(uuid))
    loadWorld(info->path);
  else
    console->error(id, "World " + to_string(uuid) + " doesn't exist");
}

void Traintastic::loadWorld(const std::filesystem::path& path)
{
  try
  {
    world = WorldLoader(path / "traintastic.json").world();
    //World::load(path / "traintastic.json");
  }
  catch(const std::exception& e)
  {
    console->critical(id, std::string("Loading world failed: ") + e.what());
  }
}

void Traintastic::saveWorld()
{
  assert(world);
  world->save();
}

void Traintastic::clientGone(const std::shared_ptr<Client>& client)
{
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
                  [this](const boost::system::error_code&, std::size_t)
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
        console->error(id, ec.message());
    });
}

std::unique_ptr<Message> Traintastic::processMessage(const Message& message)
{
  if(message.command() == Message::Command::Discover && message.isRequest())
  {
    std::unique_ptr<Message> response = Message::newResponse(message.command(), message.requestId());
    response->write(boost::asio::ip::host_name());
    return std::move(response);
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
              console->critical(id, e.what());
            }
          }
          else
            console->error(id, ec.message());
        });
    });
}
