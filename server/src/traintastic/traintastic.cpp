/**
 * server/src/traintastic/traintastic.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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
#include <traintastic/utils/str.hpp>
#include "../core/eventloop.hpp"
#include "../network/server.hpp"
#include "../core/attributes.hpp"
#include "../world/world.hpp"
#include "../world/worldlist.hpp"
#include "../world/worldloader.hpp"
#include "../log/log.hpp"
#include "../log/logmessageexception.hpp"

using nlohmann::json;

std::shared_ptr<Traintastic> Traintastic::instance;

Traintastic::Traintastic(const std::filesystem::path& dataDir) :
  m_restart{false},
  m_dataDir{std::filesystem::absolute(dataDir)},
  settings{this, "settings", nullptr, PropertyFlags::ReadWrite/*ReadOnly*/},
  world{this, "world", nullptr, PropertyFlags::ReadWrite,
    [this](const std::shared_ptr<World>& /*newWorld*/)
    {
      if(world)
        world->destroy();
      return true;
    }},
  worldList{this, "world_list", nullptr, PropertyFlags::ReadWrite/*ReadOnly*/},
  newWorld{*this, "new_world",
    [this]()
    {
#ifndef NDEBUG
      std::weak_ptr<World> weakWorld = world.value();
#endif
      world = World::create();
#ifndef NDEBUG
      assert(weakWorld.expired());
#endif
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
  closeWorld{*this, "close_world",
    [this]()
    {
#ifndef NDEBUG
      std::weak_ptr<World> weakWorld = world.value();
#endif
      world = nullptr;
#ifndef NDEBUG
      assert(weakWorld.expired());
#endif
      settings->lastWorld = "";
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
  m_interfaceItems.add(closeWorld);
  Attributes::addEnabled(restart, false);
  m_interfaceItems.add(restart);
  Attributes::addEnabled(shutdown, false);
  m_interfaceItems.add(shutdown);
}

bool Traintastic::importWorld(const std::vector<std::byte>& worldData)
{
  try
  {
#ifndef NDEBUG
    std::weak_ptr<World> weakWorld = world.value();
#endif
    world = WorldLoader(worldData).world();
#ifndef NDEBUG
    assert(weakWorld.expired());
#endif
    Log::log(*this, LogMessage::N1026_IMPORTED_WORLD_SUCCESSFULLY);
    return true;
  }
  catch(const std::exception& e)
  {
    Log::log(*this, LogMessage::C1011_IMPORTING_WORLD_FAILED_X, e.what());
  }
  return false;
}

Traintastic::RunStatus Traintastic::run()
{
  Log::log(*this, LogMessage::I1001_TRAINTASTIC_VX, std::string_view{TRAINTASTIC_VERSION_FULL});

  settings = std::make_shared<Settings>(m_dataDir);
  Attributes::setEnabled(restart, settings->allowClientServerRestart);
  Attributes::setEnabled(shutdown, settings->allowClientServerShutdown);

  worldList = std::make_shared<WorldList>(worldDir());

  if(settings->loadLastWorldOnStartup && !settings->lastWorld.value().empty())
    loadWorld(settings->lastWorld.value());

  try
  {
    m_server = std::make_shared<Server>(settings->localhostOnly, settings->port, settings->discoverable);
  }
  catch(const LogMessageException& e)
  {
    Log::log(Server::id, e.message(), e.args());
    return ExitFailure;
  }

  EventLoop::exec();

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

  EventLoop::stop();
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
#ifndef NDEBUG
    std::weak_ptr<World> weakWorld = world.value();
#endif
    world = WorldLoader(path).world();
#ifndef NDEBUG
    assert(weakWorld.expired());
#endif
    settings->lastWorld = world->uuid.value();
  }
  catch(const LogMessageException& e)
  {
    Log::log(*this, e.message(), e.args());
  }
  catch(const std::exception& e)
  {
    Log::log(*this, LogMessage::C1001_LOADING_WORLD_FAILED_X, e.what());
  }
}
